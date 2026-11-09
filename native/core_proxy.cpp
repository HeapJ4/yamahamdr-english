#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <atomic>
#include <thread>
#include "directory_ui.h"
#include "media_prepare.h"
#include "wav_export.h"
#include "runtime_log.h"

static HMODULE instance, original;
static LONG started = 0;
static std::wstring directory;
static std::string lastError;
typedef int (__cdecl *Factory)(void**,void*);
static Factory factory;
static BOOL CALLBACK ownMainWindow(HWND h,LPARAM p) {
    DWORD pid=0;GetWindowThreadProcessId(h,&pid);
    if(pid!=GetCurrentProcessId()||GetWindow(h,GW_OWNER))return TRUE;
    wchar_t name[128]={};GetClassNameW(h,name,128);
    if(wcsstr(name,L"_MainFrame")){*reinterpret_cast<HWND*>(p)=h;return FALSE;}
    return TRUE;
}
static bool nativeOpen(const std::wstring& url) {
    runtimeLog("nativeOpen requested");
    int n=WideCharToMultiByte(CP_ACP,0,url.c_str(),-1,nullptr,0,nullptr,nullptr);
    if(n<2||n>260){MessageBoxW(nullptr,L"This URL exceeds MidRadio's 259-byte native loader limit.",L"MidRadio Directory",MB_ICONERROR);return false;}
    std::string raw(n,'\0');WideCharToMultiByte(CP_ACP,0,url.c_str(),-1,&raw[0],n,nullptr,nullptr);
    // Let the original program perform its own single-instance forwarding.
    // The argument is a local playlist, not a remote URL or invented IPC ABI.
    wchar_t executable[32768] = {};
    if (!GetModuleFileNameW(nullptr, executable, 32768)) return false;
    std::wstring command = L"\"" + std::wstring(executable) + L"\" " + url;
    STARTUPINFOW startup = {}; startup.cb = sizeof(startup);
    PROCESS_INFORMATION process = {};
    std::vector<wchar_t> buffer(command.begin(), command.end()); buffer.push_back(0);
    if (!CreateProcessW(executable, buffer.data(), nullptr, nullptr, FALSE, 0,
                        nullptr, directory.c_str(), &startup, &process)) return false;
    CloseHandle(process.hThread); CloseHandle(process.hProcess);
    return true;
}

static std::wstring widen(const char* text) {
    if (!text) return L"";
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,0,0);
    UINT cp=CP_UTF8;
    if (!n) {cp=CP_ACP; n=MultiByteToWideChar(cp,0,text,-1,0,0);}
    std::wstring s(n,L'\0');
    MultiByteToWideChar(cp,0,text,-1,&s[0],n);
    if (!s.empty()) s.pop_back();
    return s;
}
static std::string utf8(const std::wstring& text) {
    int n=WideCharToMultiByte(CP_UTF8,0,text.data(),(int)text.size(),0,0,0,0);
    std::string s(n,'\0');
    WideCharToMultiByte(CP_UTF8,0,text.data(),(int)text.size(),&s[0],n,0,0);
    return s;
}
static std::string xml(const std::wstring& text) {
    std::string result;
    for (unsigned char c: utf8(text)) {
        switch(c) {
            case '&': result+="&amp;"; break;
            case '<': result+="&lt;"; break;
            case '>': result+="&gt;"; break;
            case '"': result+="&quot;"; break;
            default: if(c>=32) result+=(char)c;
        }
    }
    return result;
}

static bool launchPlaylist(const std::vector<std::pair<std::wstring,std::wstring>>& tracks,const std::wstring& title) {
    if(tracks.empty()) return false;
    wchar_t local[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,local)) return false;
    std::wstring folder=std::wstring(local)+L"MidRadio-Directory";
    CreateDirectoryW(folder.c_str(),0);
    std::wstring path=folder+L"\\playlist-"+std::to_wstring(GetCurrentProcessId())+L".mrl";
    // Direct clip children are native playlist type2. A site/channel wrapper
    // describes site configuration (type5), which the playback loader rejects.
    std::string text="<mrml version=\"1.0\">\n";
    for(auto &track:tracks) text+="<clip src=\""+xml(track.first)+"\" title=\""+xml(track.second)+"\"/>\n";
    text+="</mrml>\n";
    HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,0);
    if(file==INVALID_HANDLE_VALUE) return false;
    DWORD written=0;
    bool ok=WriteFile(file,text.data(),(DWORD)text.size(),&written,0)&&written==text.size();
    CloseHandle(file);
    if(!ok) return false;
    // MidRadio's single-instance command forwarding loads this native playlist.
    return nativeOpen(L"\""+path+L"\"");
}

static bool QueueTracks(const std::vector<std::pair<std::wstring,std::wstring>>& tracks,const std::wstring& title,const std::wstring& exportPath);
bool OpenTrackURL(const std::wstring& url,const std::wstring& name) {return QueueTracks({{url,name}},name,L"");}
bool OpenTrackPlaylist(const std::vector<std::pair<std::wstring,std::wstring>>& tracks,const std::wstring& title) {return QueueTracks(tracks,title,L"");}
bool ExportTrackWav(const std::wstring& url,const std::wstring& title,const std::wstring& output) {return QueueTracks({{url,title}},title,output);}
static bool QueueTracks(const std::vector<std::pair<std::wstring,std::wstring>>& tracks,const std::wstring& title,const std::wstring& exportPath) {
    static std::atomic<bool> busy{false};
    if(WaveExportActive()){SetDirectoryStatus(L"Finish the current WAV export before opening another track.");return false;}
    if(tracks.empty()||busy.exchange(true)){SetDirectoryStatus(L"A playback request is already being prepared.");return false;}
    try {
        std::thread([tracks,title,exportPath](){
            struct Done {~Done(){busy=false;}} done;
            try {
            std::vector<std::pair<std::wstring,std::wstring>> prepared;
            for(size_t i=0;i<tracks.size();++i){
                SetDirectoryStatus(L"Preparing "+std::to_wstring(i+1)+L"/"+std::to_wstring(tracks.size())+L": "+tracks[i].second);
                std::wstring path,error;
                if(!PrepareMedia(tracks[i].first,directory,path,error)){SetDirectoryStatus(error);return;}
                if(tracks.size()==1){
                    if(!exportPath.empty()&&!BeginWaveExport(exportPath,error)){SetDirectoryStatus(error);return;}
                    bool ok=nativeOpen(L"\""+path+L"\"");
                    if(!ok&&!exportPath.empty())FinishWaveExport();
                    SetDirectoryStatus(ok?(!exportPath.empty()?L"Rendering WAV: "+tracks[i].second+L". Click Finish export after playback.":L"Opened "+tracks[i].second):L"Could not open the prepared audio file.");return;
                }
                wchar_t fileURL[32768];DWORD size=32768;
                if(FAILED(UrlCreateFromPathW(path.c_str(),fileURL,&size,0))){SetDirectoryStatus(L"Could not create a playlist file URL.");return;}
                prepared.push_back({fileURL,tracks[i].second});
            }
            bool ok=launchPlaylist(prepared,title);
            SetDirectoryStatus(ok?L"Opened playlist: "+std::to_wstring(prepared.size())+L" tracks":L"Could not open playlist.");
            }catch(const std::exception& e){runtimeLog("media worker exception: %s",e.what());FinishWaveExport();SetDirectoryStatus(L"Audio preparation failed; see runtime.log.");}catch(...){FinishWaveExport();SetDirectoryStatus(L"Audio preparation failed unexpectedly.");}
        }).detach();
    }catch(...){busy=false;return false;}
    return true;
}

static int __fastcall HtmlBridge(void*,void*,const char* url,const char*) {
    if(url && *url) NavigateDirectory(widen(url));
    return 0;
}
static void patchHTML() {
    unsigned char* address=(unsigned char*)original+0x1130;
    const unsigned char expected[]={0x56,0x57,0x8b,0x7c,0x24,0x0c,0x8b,0xf1};
    if(memcmp(address,expected,sizeof(expected))!=0) {
        MessageBoxW(0,L"This PlayerCore.dll build does not match the directory extension. Browser interception was not enabled.",L"MidRadio Directory",MB_ICONERROR);
        return;
    }
    DWORD old;
    if(!VirtualProtect(address,5,PAGE_EXECUTE_READWRITE,&old)) return;
    address[0]=0xe9;
    *(DWORD*)(address+1)=(DWORD)((unsigned char*)&HtmlBridge-address-5);
    FlushInstructionCache(GetCurrentProcess(),address,5);
    DWORD ignored; VirtualProtect(address,5,old,&ignored);
}

extern "C" __declspec(dllexport) int __cdecl MRACreateInstance(void** out,void* context) {
    SetUnhandledExceptionFilter(runtimeCrash);
    runtimeLog("core factory pid=%lu context=%p",GetCurrentProcessId(),context);
    if(!original) {
        // The native loader releases its factory DLL after obtaining the core
        // object. Our UI thread and browser hook must live for the process.
        HMODULE pinned=nullptr;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                          reinterpret_cast<LPCWSTR>(&MRACreateInstance),&pinned);
        wchar_t path[32768]; GetModuleFileNameW(instance,path,32768);
        PathRemoveFileSpecW(path); directory=path;
        original=LoadLibraryW((directory+L"\\CoreOriginal.dll").c_str());
        if(!original) {if(out)*out=0; return 8;}
        factory=(Factory)GetProcAddress(original,"MRACreateInstance");
        if(!factory) {if(out)*out=0; return 8;}
        patchHTML();
        InitializeWaveExport(directory);
    }
    int result=factory(out,context);
    runtimeLog("core factory result=%d object=%p",result,out?*out:nullptr);
    if(InterlockedCompareExchange(&started,1,0)==0) StartDirectoryUI(instance);
    return result;
}

BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH) {instance=h; DisableThreadLibraryCalls(h);}
    return TRUE;
}
