#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <shlwapi.h>
#include <cwctype>
#include <cstdio>
#include <cstdint>
#include <vector>
#include "media_prepare.h"

namespace {
constexpr uint64_t MAX_INPUT=256ULL*1024*1024;
struct Internet {HINTERNET h=nullptr;~Internet(){if(h)InternetCloseHandle(h);}};
std::wstring Extension(const std::wstring& value){
    std::wstring path=value.substr(0,value.find_first_of(L"?#"));
    auto dot=path.find_last_of(L'.');auto slash=path.find_last_of(L"/\\");
    if(dot==path.npos||(slash!=path.npos&&dot<slash))return L"";
    auto ext=path.substr(dot);for(auto& c:ext)c=towlower(c);return ext;
}
bool Download(const std::wstring& url,const std::wstring& output,std::wstring& error){
    Internet session;session.h=InternetOpenW(L"MidRadio Directory/2.0",INTERNET_OPEN_TYPE_PRECONFIG,nullptr,nullptr,0);
    if(!session.h){error=L"Could not initialize the download.";return false;}
    DWORD timeout=15000;
    InternetSetOptionW(session.h,INTERNET_OPTION_CONNECT_TIMEOUT,&timeout,sizeof(timeout));
    InternetSetOptionW(session.h,INTERNET_OPTION_RECEIVE_TIMEOUT,&timeout,sizeof(timeout));
    Internet page;page.h=InternetOpenUrlW(session.h,url.c_str(),nullptr,0,INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_UI,0);
    if(!page.h){error=L"Could not download this track.";return false;}
    DWORD status=0,length=sizeof(status);
    if(!HttpQueryInfoW(page.h,HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,&status,&length,nullptr)||status<200||status>=300){error=L"Track server returned HTTP "+std::to_wstring(status);return false;}
    HANDLE file=CreateFileW(output.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,nullptr);
    if(file==INVALID_HANDLE_VALUE){error=L"Could not create the local audio cache.";return false;}
    bool ok=true;uint64_t total=0;char buffer[32768];DWORD got=0,written=0;
    ULONGLONG deadline=GetTickCount64()+180000;
    for(;;){
        if(GetTickCount64()>deadline||!InternetReadFile(page.h,buffer,sizeof(buffer),&got)){error=L"Track download failed or timed out.";ok=false;break;}
        if(!got)break;
        total+=got;
        if(total>MAX_INPUT){error=L"Track exceeds the 256 MiB download limit.";ok=false;break;}
        if(!WriteFile(file,buffer,got,&written,nullptr)||written!=got){error=L"Could not write the local audio cache.";ok=false;break;}
    }
    CloseHandle(file);
    if(!total){error=L"The track is empty.";ok=false;}
    if(!ok)DeleteFileW(output.c_str());
    return ok;
}
}

bool PrepareMedia(const std::wstring& source,const std::wstring& folder,std::wstring& localFile,std::wstring& error){
    auto ext=Extension(source);
    bool converted=ext==L".ogg"||ext==L".m4a";
    bool remote=source.find(L"http://")==0||source.find(L"https://")==0;
    wchar_t temp[MAX_PATH]={};if(!GetTempPathW(MAX_PATH,temp)){error=L"Temporary directory unavailable.";return false;}
    std::wstring cache=std::wstring(temp)+L"MidRadio-Directory-cache";
    if(!CreateDirectoryW(cache.c_str(),nullptr)&&GetLastError()!=ERROR_ALREADY_EXISTS){error=L"Could not create the audio cache directory.";return false;}
    uint64_t hash=14695981039346656037ULL;
    for(wchar_t c:source){hash^=uint16_t(c);hash*=1099511628211ULL;}
    wchar_t key[32];swprintf(key,32,L"%016llx",(unsigned long long)hash);
    std::wstring input=remote?cache+L"\\"+key+ext:source;
    std::wstring output=cache+L"\\"+key+L"-decoded.wav";
    if(remote&&GetFileAttributesW(input.c_str())==INVALID_FILE_ATTRIBUTES){
        if(!Download(source,input,error))return false;
    }
    if(!converted){localFile=input;return true;}
    if(GetFileAttributesW(output.c_str())!=INVALID_FILE_ATTRIBUTES){localFile=output;return true;}
    std::wstring decoder=folder+L"\\MidRadioDecode.exe";
    std::wstring command=L"\""+decoder+L"\" \""+input+L"\" \""+output+L"\"";
    std::vector<wchar_t> text(command.begin(),command.end());text.push_back(0);
    STARTUPINFOW startup={};startup.cb=sizeof(startup);PROCESS_INFORMATION process={};
    if(!CreateProcessW(decoder.c_str(),text.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,folder.c_str(),&startup,&process)){error=L"The audio decoder could not be started. Keep MidRadioDecode.exe next to the player.";return false;}
    DWORD wait=WaitForSingleObject(process.hProcess,180000),code=1;
    if(wait==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&code);
    else {TerminateProcess(process.hProcess,1);WaitForSingleObject(process.hProcess,5000);}
    CloseHandle(process.hThread);CloseHandle(process.hProcess);
    if(code!=0){DeleteFileW(output.c_str());error=L"Audio decode failed. OGG needs Vorbis; M4A needs a Windows-supported codec.";return false;}
    localFile=output;return true;
}
