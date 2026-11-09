#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include "directory_ui.h"
#include "skin_english.h"
#include "wav_export.h"
#include <commdlg.h>
#include <commctrl.h>
#include <windowsx.h>
#include <wininet.h>
#include <shlwapi.h>
#include <algorithm>
#include <cwctype>
#include <memory>
#include <set>
#include <atomic>
#include "runtime_log.h"

namespace {
constexpr UINT WM_RESULT=WM_APP+421, WM_NAVIGATE=WM_APP+422, WM_PARENT_LAYOUT=WM_APP+423, WM_STATUS=WM_APP+425;
constexpr int ID_ADDRESS=1101, ID_GO=1102, ID_UP=1103, ID_SEARCH=1104,
              ID_LIST=1105, ID_PLAY=1106, ID_STATUS=1107, ID_PLAY_SELECTED=1108, ID_EXPORT=1109, ID_FINISH=1110;
constexpr size_t MAX_HTML=4*1024*1024, MAX_ENTRIES=20000;
const wchar_t CLASS_NAME[]=L"MidRadioDirectoryPaneV1";
struct Entry {std::wstring title,url; bool directory=false;};
struct Result {unsigned generation; std::wstring url,error; std::vector<Entry> entries;};
struct Request { HWND window; unsigned generation; std::wstring url;};
HMODULE moduleHandle=nullptr;
HWND pane=nullptr,mainWindow=nullptr,address=nullptr,searchBox=nullptr,list=nullptr,status=nullptr;
HFONT font=nullptr;
int originalHeight=0;
unsigned generation=0;
const UINT WM_INITIALIZE=RegisterWindowMessageW(L"HeapJ4.MidRadio.Directory.Initialize.1");
HHOOK initializeHook=nullptr;
bool adjustingFrame=false,interactiveResize=false;
int reservedHeight=0;
using ClientRectFn=BOOL(WINAPI*)(HWND,LPRECT);
ClientRectFn nativeClientRect=nullptr;
BOOL WINAPI YamahaClientRect(HWND h,LPRECT r){
 BOOL ok=nativeClientRect(h,r);
 if(ok&&h==mainWindow&&reservedHeight)r->bottom=std::max(r->top,r->bottom-reservedHeight);
 return ok;
}
void HookClientRect(){
 auto base=reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr));
 auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
 auto nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);
 auto rva=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
 if(!rva)return;
 auto libs=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+rva);
 for(;libs->Name;++libs){
  if(_stricmp(reinterpret_cast<char*>(base+libs->Name),"user32.dll")||!libs->OriginalFirstThunk)continue;
  auto names=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+libs->OriginalFirstThunk);
  auto slots=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+libs->FirstThunk);
  for(;names->u1.AddressOfData;++names,++slots){
   if(IMAGE_SNAP_BY_ORDINAL32(names->u1.Ordinal))continue;
   auto name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData);
   if(strcmp(reinterpret_cast<char*>(name->Name),"GetClientRect"))continue;
   if(slots->u1.Function==reinterpret_cast<DWORD>(YamahaClientRect))return;
   DWORD old=0;if(!VirtualProtect(&slots->u1.Function,4,PAGE_READWRITE,&old))return;
   nativeClientRect=reinterpret_cast<ClientRectFn>(slots->u1.Function);
   InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&slots->u1.Function),reinterpret_cast<void*>(YamahaClientRect));
   DWORD ignored;VirtualProtect(&slots->u1.Function,4,old,&ignored);return;
  }
 }
}
std::vector<Entry> entries;
std::vector<size_t> visible;
std::wstring currentURL;
SRWLOCK pendingLock=SRWLOCK_INIT;
std::wstring pendingURL;
std::atomic<bool> started{false};
std::atomic<unsigned> outstanding{0};

std::wstring Lower(std::wstring s) {std::transform(s.begin(),s.end(),s.begin(),[](wchar_t c){return static_cast<wchar_t>(towlower(c));});return s;}
std::wstring Trim(std::wstring s) {auto b=s.find_first_not_of(L" \t\r\n");if(b==s.npos)return L"";return s.substr(b,s.find_last_not_of(L" \t\r\n")-b+1);}
std::wstring Text(HWND h) {int n=GetWindowTextLengthW(h);std::wstring s(n+1,L'\0');GetWindowTextW(h,&s[0],n+1);s.resize(n);return s;}
void Status(const std::wstring& s) {SetWindowTextW(status,s.c_str());}
std::wstring DecodeEntities(const std::wstring& s) {
 std::wstring out;
 for(size_t i=0;i<s.size();++i){
  if(s[i]!=L'&'){out+=s[i];continue;}
  size_t end=s.find(L';',i+1);if(end==s.npos||end-i>14){out+=s[i];continue;}
  std::wstring e=s.substr(i+1,end-i-1);unsigned long cp=0;
  if(e==L"amp")cp='&';else if(e==L"quot")cp='"';else if(e==L"apos"||e==L"#39")cp='\'';
  else if(e==L"lt")cp='<';else if(e==L"gt")cp='>';else if(e==L"nbsp")cp=' ';
  else if(e.size()>1&&e[0]==L'#') {wchar_t* tail=nullptr; bool hex=e.size()>2&&(e[1]==L'x'||e[1]==L'X');cp=wcstoul(e.c_str()+(hex?2:1),&tail,hex?16:10);if(!tail||*tail)cp=0;}
  if(cp&&cp<=0x10ffff&&!(cp>=0xd800&&cp<=0xdfff)){if(cp>0xffff){cp-=0x10000;out+=wchar_t(0xd800+(cp>>10));out+=wchar_t(0xdc00+(cp&1023));}else out+=wchar_t(cp);i=end;}else out+=s[i];
 }
 return out;
}
std::wstring PlainText(const std::wstring& s){std::wstring out;bool tag=false;for(wchar_t c:s){if(c==L'<')tag=true;else if(c==L'>')tag=false;else if(!tag)out+=c;}return Trim(DecodeEntities(out));}
std::wstring PercentDecode(const std::wstring& s){
 // Decode UTF-8 percent escapes for display only. URLs retain their escaped bytes.
 std::wstring out;
 for(size_t i=0;i<s.size();){
  if(s[i]!=L'%'){out+=s[i++];continue;}
  std::string bytes;size_t j=i;
  while(j+2<s.size()&&s[j]==L'%'){auto hex=[](wchar_t c)->int{if(c>=L'0'&&c<=L'9')return c-L'0';if(c>=L'a'&&c<=L'f')return c-L'a'+10;if(c>=L'A'&&c<=L'F')return c-L'A'+10;return -1;};int a=hex(s[j+1]),b=hex(s[j+2]);if(a<0||b<0)break;bytes+=char(a*16+b);j+=3;}
  if(bytes.empty()){out+=s[i++];continue;}int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),int(bytes.size()),nullptr,0);if(n){std::wstring w(n,L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),int(bytes.size()),&w[0],n);out+=w;}else out+=s.substr(i,j-i);i=j;
 }return out;
}
std::wstring Resolve(const std::wstring& base,const std::wstring& ref){DWORD n=32768;std::wstring u(n,L'\0');if(!InternetCombineUrlW(base.c_str(),ref.c_str(),&u[0],&n,ICU_NO_ENCODE))return L"";u.resize(n);if(!u.empty()&&u.back()==L'\0')u.pop_back();size_t hash=u.find(L'#');if(hash!=u.npos)u.resize(hash);std::wstring low=Lower(u);if(low.find(L"http://")!=0&&low.find(L"https://")!=0)return L"";return u;}
std::wstring PathPart(const std::wstring& url){URL_COMPONENTSW c={};c.dwStructSize=sizeof(c);c.dwUrlPathLength=DWORD(-1);c.dwExtraInfoLength=DWORD(-1);if(!InternetCrackUrlW(url.c_str(),0,0,&c))return L"";return std::wstring(c.lpszUrlPath,c.dwUrlPathLength);}
bool Supported(const std::wstring& url){std::wstring p=Lower(PercentDecode(PathPart(url)));size_t dot=p.rfind(L'.');if(dot==p.npos)return false;auto e=p.substr(dot);return e==L".mmf"||e==L".mp3"||e==L".wav"||e==L".ogg"||e==L".mid"||e==L".midi"||e==L".mrm"||e==L".mrl"||e==L".m4a";}
std::wstring Attribute(const std::wstring& tag,const std::wstring& name){
 size_t i=1;while(i<tag.size()&&!iswspace(tag[i])&&tag[i]!=L'>')++i;
 while(i<tag.size()){while(i<tag.size()&&(iswspace(tag[i])||tag[i]==L'/'))++i;size_t b=i;while(i<tag.size()&&!iswspace(tag[i])&&tag[i]!=L'='&&tag[i]!=L'>')++i;if(i==b){++i;continue;}std::wstring key=Lower(tag.substr(b,i-b));while(i<tag.size()&&iswspace(tag[i]))++i;if(i>=tag.size()||tag[i]!=L'=')continue;++i;while(i<tag.size()&&iswspace(tag[i]))++i;wchar_t quote=(i<tag.size()&&(tag[i]==L'\''||tag[i]==L'"'))?tag[i++]:0;b=i;if(quote){while(i<tag.size()&&tag[i]!=quote)++i;}else{while(i<tag.size()&&!iswspace(tag[i])&&tag[i]!=L'>')++i;}std::wstring value=tag.substr(b,i-b);if(quote&&i<tag.size())++i;if(key==name)return DecodeEntities(value);}
 return L"";
}
std::vector<Entry> Parse(const std::wstring& html,const std::wstring& url){
 std::wstring low=Lower(html),base=url;std::vector<Entry> out;std::set<std::wstring> seen;
 size_t bp=low.find(L"<base");if(bp!=low.npos){size_t e=low.find(L'>',bp);if(e!=low.npos){auto href=Attribute(html.substr(bp,e-bp+1),L"href");auto resolved=Resolve(url,href);if(!href.empty()&&!resolved.empty())base=resolved;}}
 for(size_t pos=0;out.size()<MAX_ENTRIES;){pos=low.find(L"<a",pos);if(pos==low.npos)break;size_t b=pos;pos+=2;if(pos<low.size()&&!iswspace(low[pos])&&low[pos]!=L'>')continue;size_t end=pos;wchar_t quote=0;for(;end<html.size();++end){wchar_t c=html[end];if(quote){if(c==quote)quote=0;}else if(c==L'\''||c==L'"')quote=c;else if(c==L'>')break;}if(end>=html.size())break;pos=end+1;auto href=Trim(Attribute(html.substr(b,end-b+1),L"href"));if(href.empty()||href[0]==L'#'||href[0]==L'?')continue;std::wstring target=Resolve(base,href);if(target.empty()||!seen.insert(target).second)continue;auto path=PathPart(target);bool dir=!path.empty()&&path.back()==L'/';if(!dir&&!Supported(target))continue;size_t close=low.find(L"</a",pos);std::wstring title=close==low.npos?L"":PlainText(html.substr(pos,std::min<size_t>(close-pos,4096)));if(title.empty()){auto p=path;if(dir&&!p.empty())p.pop_back();size_t slash=p.rfind(L'/');title=PercentDecode(slash==p.npos?p:p.substr(slash+1));if(dir)title+=L"/";}if(title.empty())title=L"/";out.push_back({title,target,dir});}
 std::stable_sort(out.begin(),out.end(),[](const Entry&a,const Entry&b){if(a.directory!=b.directory)return a.directory;return StrCmpLogicalW(a.title.c_str(),b.title.c_str())<0;});return out;
}
struct InternetHandle {HINTERNET h;explicit InternetHandle(HINTERNET p=nullptr):h(p){}~InternetHandle(){if(h)InternetCloseHandle(h);}operator HINTERNET()const{return h;}};
DWORD WINAPI Fetch(void* ptr){
 struct Completion {~Completion(){--outstanding;}} completion;
 const ULONGLONG deadline=GetTickCount64()+45000;
 std::unique_ptr<Request> req(static_cast<Request*>(ptr));std::unique_ptr<Result> r(new Result);r->generation=req->generation;r->url=req->url;
 try {
 InternetHandle session(InternetOpenW(L"MidRadio Directory Browser/1.0",INTERNET_OPEN_TYPE_PRECONFIG,nullptr,nullptr,0));
 if(!session)r->error=L"Could not initialize the network connection.";
 else {DWORD timeout=15000;InternetSetOptionW(session,INTERNET_OPTION_CONNECT_TIMEOUT,&timeout,sizeof(timeout));InternetSetOptionW(session,INTERNET_OPTION_RECEIVE_TIMEOUT,&timeout,sizeof(timeout));InternetSetOptionW(session,INTERNET_OPTION_SEND_TIMEOUT,&timeout,sizeof(timeout));
  InternetHandle page(InternetOpenUrlW(session,req->url.c_str(),L"Accept: text/html,application/xhtml+xml\r\n",DWORD(-1),INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_UI,0));
  if(!page)r->error=L"Connection failed (Windows error "+std::to_wstring(GetLastError())+L"). Check the URL and connection.";
  else {DWORD code=0,n=sizeof(code);HttpQueryInfoW(page,HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,&code,&n,nullptr);
   if(code<200||code>=300)r->error=L"Server returned HTTP "+std::to_wstring(code)+L".";
   else {wchar_t finalURL[32768];n=sizeof(finalURL);if(InternetQueryOptionW(page,INTERNET_OPTION_URL,finalURL,&n))r->url=finalURL;
    std::string bytes;char buf[16384];DWORD got=0;bool ok=true;while(true){if(GetTickCount64()>deadline){r->error=L"Directory request timed out.";break;}if(!InternetReadFile(page,buf,sizeof(buf),&got)){ok=false;break;}if(!got)break;if(bytes.size()+got>MAX_HTML){r->error=L"Directory page exceeds the 4 MB limit.";break;}bytes.append(buf,got);}
    if(!ok)r->error=L"Download interrupted (Windows error "+std::to_wstring(GetLastError())+L").";
    if(r->error.empty()){int skip=bytes.size()>=3&&static_cast<unsigned char>(bytes[0])==0xef&&static_cast<unsigned char>(bytes[1])==0xbb&&static_cast<unsigned char>(bytes[2])==0xbf?3:0;int count=int(bytes.size())-skip;int chars=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data()+skip,count,nullptr,0);UINT cp=CP_UTF8;if(!chars&&count){cp=1252;chars=MultiByteToWideChar(cp,0,bytes.data()+skip,count,nullptr,0);}std::wstring html(chars,L'\0');if(chars)MultiByteToWideChar(cp,0,bytes.data()+skip,count,&html[0],chars);r->entries=Parse(html,r->url);}
   }
  }
 }
 }catch(const std::exception& e){runtimeLog("directory worker exception: %s",e.what());r->error=L"Directory request failed while processing the response.";}catch(...){r->error=L"Directory request failed unexpectedly.";}
 if(IsWindow(req->window)&&PostMessageW(req->window,WM_RESULT,0,reinterpret_cast<LPARAM>(r.get())))r.release();
 return 0;
}
void Refill(){ListView_DeleteAllItems(list);visible.clear();auto query=Lower(Text(searchBox));for(size_t i=0;i<entries.size();++i){auto& e=entries[i];if(!query.empty()&&Lower(e.title).find(query)==std::wstring::npos)continue;int row=int(visible.size());visible.push_back(i);LVITEMW item={};item.mask=LVIF_TEXT;item.iItem=row;item.pszText=const_cast<wchar_t*>(e.title.c_str());ListView_InsertItem(list,&item);ListView_SetItemText(list,row,1,const_cast<wchar_t*>(e.directory?L"Folder":PathFindExtensionW(PercentDecode(PathPart(e.url)).c_str())));ListView_SetItemText(list,row,2,const_cast<wchar_t*>(e.url.c_str()));}Status(std::to_wstring(visible.size())+L" items shown · Double-click a folder to browse or a track to play");}
void Navigate(const std::wstring& raw){std::wstring url=Trim(raw);if(url.empty())return;auto l=Lower(url);if(l.find(L"http://")!=0&&l.find(L"https://")!=0){Status(L"Enter an http:// or https:// directory URL.");return;}if(outstanding.load()>=4){Status(L"Four requests are already in progress. Try again when one finishes.");return;}++outstanding;SetWindowTextW(address,url.c_str());Status(L"Loading directory…");auto req=new Request{pane,++generation,url};HANDLE h=CreateThread(nullptr,0,Fetch,req,0,nullptr);if(h)CloseHandle(h);else{--outstanding;delete req;Status(L"Could not start the directory request.");}}
void Activate(){int row=ListView_GetNextItem(list,-1,LVNI_SELECTED);if(row<0||size_t(row)>=visible.size())return;auto e=entries[visible[row]];if(e.directory)Navigate(e.url);else if(OpenTrackURL(e.url,e.title))Status(L"Opening "+e.title);else Status(L"Could not open "+e.title+L". Check the playback status.");}
void PlaySelected(){int row=ListView_GetNextItem(list,-1,LVNI_SELECTED);if(row<0||size_t(row)>=visible.size()||entries[visible[row]].directory){Status(L"Select an audio file, then click Play selected.");return;}Activate();}
void PlayAll(){std::vector<std::pair<std::wstring,std::wstring>> tracks;for(size_t i:visible)if(!entries[i].directory)tracks.push_back({entries[i].url,entries[i].title});if(tracks.empty()){Status(L"No supported tracks in the current view.");return;}if(OpenTrackPlaylist(tracks,currentURL))Status(L"Loaded "+std::to_wstring(tracks.size())+L" tracks into the playlist.");else Status(L"Could not load the playlist.");}
void ExportSelected(){
 int row=ListView_GetNextItem(list,-1,LVNI_SELECTED);
 if(row<0||size_t(row)>=visible.size()||entries[visible[row]].directory){Status(L"Select a track to render, then click Export WAV.");return;}
 auto entry=entries[visible[row]];std::wstring suggested=entry.title;
 for(auto& c:suggested)if(c<32||wcschr(L"<>:\"/\\|?*",c))c=L'_';
 auto dot=suggested.find_last_of(L'.');if(dot!=suggested.npos)suggested.resize(dot);
 if(suggested.empty())suggested=L"MidRadio export";
 wchar_t path[MAX_PATH]={};wcsncpy(path,(suggested.substr(0,180)+L".wav").c_str(),MAX_PATH-1);
 OPENFILENAMEW dialog={};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=pane;
 dialog.lpstrFilter=L"PCM WAV audio (*.wav)\0*.wav\0\0";dialog.lpstrFile=path;dialog.nMaxFile=MAX_PATH;
 dialog.lpstrDefExt=L"wav";dialog.lpstrTitle=L"Render selected track to WAV";
 dialog.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(!GetSaveFileNameW(&dialog))return;
 if(_wcsicmp(PathFindExtensionW(path),L".wav")){Status(L"Choose a destination ending in .wav.");return;}
 if(ExportTrackWav(entry.url,entry.title,path))Status(L"Preparing WAV export. Use the built-in Yamaha software synthesizer.");
}
void CopyLink(){int row=ListView_GetNextItem(list,-1,LVNI_SELECTED);if(row<0||size_t(row)>=visible.size())return;auto& u=entries[visible[row]].url;if(!OpenClipboard(pane))return;HGLOBAL mem=GlobalAlloc(GMEM_MOVEABLE,(u.size()+1)*sizeof(wchar_t));if(mem){void* p=GlobalLock(mem);if(p){memcpy(p,u.c_str(),(u.size()+1)*sizeof(wchar_t));GlobalUnlock(mem);EmptyClipboard();if(!SetClipboardData(CF_UNICODETEXT,mem))GlobalFree(mem);}else GlobalFree(mem);}CloseClipboard();}
void Layout(){RECT r;GetClientRect(pane,&r);int w=r.right,h=r.bottom;int pad=10,gap=6;int addrWidth=std::max(50,w-216);MoveWindow(GetDlgItem(pane,ID_UP),pad,pad,40,26,TRUE);MoveWindow(address,pad+46,pad,addrWidth,26,TRUE);MoveWindow(GetDlgItem(pane,ID_GO),w-104,pad,94,26,TRUE);MoveWindow(searchBox,pad,42,std::max(50,w-500),25,TRUE);MoveWindow(GetDlgItem(pane,ID_PLAY_SELECTED),w-484,42,116,25,TRUE);MoveWindow(GetDlgItem(pane,ID_PLAY),w-362,42,94,25,TRUE);MoveWindow(GetDlgItem(pane,ID_EXPORT),w-262,42,116,25,TRUE);MoveWindow(GetDlgItem(pane,ID_FINISH),w-140,42,130,25,TRUE);MoveWindow(list,pad,74,std::max(30,w-pad*2),std::max(30,h-104),TRUE);MoveWindow(status,pad,std::max(110,h-25),std::max(30,w-pad*2),20,TRUE);ListView_SetColumnWidth(list,0,std::max(120,w*45/100));ListView_SetColumnWidth(list,1,60);ListView_SetColumnWidth(list,2,std::max(120,w*50/100-90));(void)gap;}
LRESULT CALLBACK EditProc(HWND h,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR){if(msg==WM_GETDLGCODE&&wp==VK_RETURN)return DLGC_WANTALLKEYS;if(msg==WM_KEYDOWN&&wp==VK_RETURN){if(h==address)Navigate(Text(address));else if(h==list)Activate();return 0;}if(msg==WM_NCDESTROY)RemoveWindowSubclass(h,EditProc,id);return DefSubclassProc(h,msg,wp,lp);}
void InitializePane(HWND target);
LRESULT CALLBACK ParentProc(HWND h,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR){
 if(msg==WM_ENTERSIZEMOVE)interactiveResize=true;
 if(msg==WM_EXITSIZEMOVE)interactiveResize=false;
 if(msg==WM_WINDOWPOSCHANGING&&pane&&!adjustingFrame&&!interactiveResize){
  auto pos=reinterpret_cast<WINDOWPOS*>(lp);
  if(!(pos->flags&SWP_NOSIZE)){
   RECT current={};GetWindowRect(h,&current);
   if(pos->cy!=current.bottom-current.top||pos->cx!=current.right-current.left){
    // Yamaha requests a native skin size. Reserve additional space exactly once.
    reservedHeight=pos->cx>=660?340:0;
    pos->cy+=reservedHeight;
   }
  }
 }
 LPARAM nativeSize=lp;
 if(msg==WM_SIZE&&wp!=SIZE_MINIMIZED&&reservedHeight)
  nativeSize=MAKELPARAM(LOWORD(lp),std::max(0,int(HIWORD(lp))-reservedHeight));
 LRESULT ret=DefSubclassProc(h,msg,wp,nativeSize);
 if(msg==WM_SIZE&&pane&&wp!=SIZE_MINIMIZED){
  RECT r={};GetClientRect(h,&r);
  bool full=reservedHeight&&r.right>=660;
  ShowWindow(pane,full?SW_SHOWNOACTIVATE:SW_HIDE);
  if(full)PostMessageW(pane,WM_PARENT_LAYOUT,WPARAM(std::max(0,int(r.bottom)-reservedHeight)),MAKELPARAM(r.right,reservedHeight));
  runtimeLog("layout mode client=%ldx%ld reserved=%d",r.right,r.bottom,reservedHeight);
 }
 if(msg==WM_NCDESTROY){RemoveWindowSubclass(h,ParentProc,id);mainWindow=nullptr;reservedHeight=0;}
 return ret;
}
LRESULT CALLBACK InitializeOnGUIThread(int code,WPARAM wp,LPARAM lp){
 if(code>=0){auto m=reinterpret_cast<CWPSTRUCT*>(lp);
  if(m->message==WM_INITIALIZE&&m->hwnd==mainWindow){
   if(SetWindowSubclass(mainWindow,ParentProc,0x4d524431,0))InitializePane(mainWindow);
  }
 }
 return CallNextHookEx(initializeHook,code,wp,lp);
}
LRESULT CALLBACK PaneProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){
 switch(msg){
 case WM_CREATE:{pane=h;auto make=[&](const wchar_t*c,const wchar_t*t,DWORD style,int id){HWND v=CreateWindowExW((wcscmp(c,L"EDIT")==0||wcscmp(c,WC_LISTVIEWW)==0)?WS_EX_CLIENTEDGE:0,c,t,WS_CHILD|WS_VISIBLE|style,0,0,1,1,h,HMENU(INT_PTR(id)),moduleHandle,nullptr);SendMessageW(v,WM_SETFONT,WPARAM(font),TRUE);return v;};make(L"BUTTON",L"Up",BS_PUSHBUTTON,ID_UP);address=make(L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,ID_ADDRESS);SendMessageW(address,EM_SETCUEBANNER,TRUE,LPARAM(L"https://3.onj.me/phonetones/unzipped/Samsung/SGH-D500/"));make(L"BUTTON",L"Open folder",BS_PUSHBUTTON|WS_TABSTOP,ID_GO);searchBox=make(L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,ID_SEARCH);SendMessageW(searchBox,EM_SETCUEBANNER,TRUE,LPARAM(L"Filter files and folders"));make(L"BUTTON",L"Play selected",BS_PUSHBUTTON|WS_TABSTOP,ID_PLAY_SELECTED);make(L"BUTTON",L"Play all",BS_PUSHBUTTON|WS_TABSTOP,ID_PLAY);make(L"BUTTON",L"Export WAV",BS_PUSHBUTTON|WS_TABSTOP,ID_EXPORT);make(L"BUTTON",L"Finish export",BS_PUSHBUTTON|WS_TABSTOP,ID_FINISH);list=make(WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS|WS_TABSTOP,ID_LIST);ListView_SetExtendedListViewStyle(list,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);LVCOLUMNW col={};col.mask=LVCF_TEXT|LVCF_WIDTH;col.cx=220;col.pszText=const_cast<wchar_t*>(L"Name");ListView_InsertColumn(list,0,&col);col.cx=60;col.pszText=const_cast<wchar_t*>(L"Type");ListView_InsertColumn(list,1,&col);col.cx=280;col.pszText=const_cast<wchar_t*>(L"Address");ListView_InsertColumn(list,2,&col);status=make(L"STATIC",L"Web directory · Yamaha, WAV, MP3, OGG, M4A",SS_LEFT,ID_STATUS);SetWindowSubclass(address,EditProc,1,0);SetWindowSubclass(list,EditProc,2,0);Layout();return 0;}
 case WM_SIZE:Layout();return 0;
 case WM_PARENT_LAYOUT:MoveWindow(h,0,int(wp),LOWORD(lp),HIWORD(lp),TRUE);return 0;
 case WM_TIMER:if(wp==1){bool exporting=WaveExportActive();EnableWindow(GetDlgItem(h,ID_EXPORT),!exporting);EnableWindow(GetDlgItem(h,ID_FINISH),exporting);}return 0;
 case WM_COMMAND:if(LOWORD(wp)==ID_GO)Navigate(Text(address));else if(LOWORD(wp)==ID_UP){auto u=Resolve(currentURL.empty()?Text(address):currentURL,L"../");if(!u.empty())Navigate(u);}else if(LOWORD(wp)==ID_PLAY)PlayAll();else if(LOWORD(wp)==ID_PLAY_SELECTED)PlaySelected();else if(LOWORD(wp)==ID_EXPORT)ExportSelected();else if(LOWORD(wp)==ID_FINISH)FinishWaveExport();else if(LOWORD(wp)==ID_SEARCH&&HIWORD(wp)==EN_CHANGE)Refill();return 0;
 case WM_NOTIFY:{auto nm=reinterpret_cast<NMHDR*>(lp);if(nm->idFrom==ID_LIST){if(nm->code==NM_DBLCLK){Activate();return 0;}if(nm->code==LVN_KEYDOWN&&reinterpret_cast<NMLVKEYDOWN*>(lp)->wVKey==VK_RETURN){Activate();return 0;}}break;}
 case WM_CONTEXTMENU:if(HWND(wp)==list){POINT pt{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};if(pt.x==-1){GetCursorPos(&pt);}else{POINT local=pt;ScreenToClient(list,&local);LVHITTESTINFO hit={};hit.pt=local;int row=ListView_HitTest(list,&hit);if(row>=0)ListView_SetItemState(list,row,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);}HMENU menu=CreatePopupMenu();AppendMenuW(menu,MF_STRING,1,L"Open / Play");AppendMenuW(menu,MF_STRING,2,L"Copy full URL");int cmd=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON,pt.x,pt.y,0,h,nullptr);DestroyMenu(menu);if(cmd==1)Activate();if(cmd==2)CopyLink();return 0;}break;
 case WM_NAVIGATE:{std::unique_ptr<std::wstring> u(reinterpret_cast<std::wstring*>(lp));Navigate(*u);return 0;}
 case WM_STATUS:{std::unique_ptr<std::wstring> text(reinterpret_cast<std::wstring*>(lp));Status(*text);return 0;}
 case WM_RESULT:{std::unique_ptr<Result> r(reinterpret_cast<Result*>(lp));if(r->generation!=generation)return 0;if(!r->error.empty()){Status(r->error);return 0;}currentURL=r->url;entries=std::move(r->entries);SetWindowTextW(address,currentURL.c_str());Refill();if(entries.empty())Status(L"No folders or supported audio links found in this directory.");return 0;}
 case WM_DESTROY:KillTimer(h,1);pane=nullptr;if(font){DeleteObject(font);font=nullptr;}return 0;
 }
 return DefWindowProcW(h,msg,wp,lp);
}
BOOL CALLBACK FindMain(HWND h,LPARAM param){DWORD pid=0;GetWindowThreadProcessId(h,&pid);if(pid==GetCurrentProcessId()&&IsWindowVisible(h)&&!GetWindow(h,GW_OWNER)){wchar_t title[256]={};GetWindowTextW(h,title,256);std::wstring lower=Lower(title);wchar_t className[256]={};GetClassNameW(h,className,256);if(lower.find(L"midradio")!=std::wstring::npos&&wcsstr(className,L"_MainFrame")){*reinterpret_cast<HWND*>(param)=h;return FALSE;}}return TRUE;}
void InitializePane(HWND target){
 if(pane||!IsWindow(target))return;
 // All widgets are created on MidRadio's GUI thread and share its input queue.
 InitCommonControls();
 NONCLIENTMETRICSW ncm={};ncm.cbSize=sizeof(ncm);
 SystemParametersInfoW(SPI_GETNONCLIENTMETRICS,sizeof(ncm),&ncm,0);
 font=CreateFontIndirectW(&ncm.lfMessageFont);
 WNDCLASSW cls={};cls.lpfnWndProc=PaneProc;cls.hInstance=moduleHandle;
 cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hbrBackground=GetSysColorBrush(COLOR_BTNFACE);
 cls.lpszClassName=CLASS_NAME;RegisterClassW(&cls);
 RECT client,window;GetClientRect(target,&client);GetWindowRect(target,&window);
 originalHeight=client.bottom;
 HookClientRect();
 reservedHeight=client.right>=660?340:0;
 HWND child=CreateWindowExW(WS_EX_CONTROLPARENT,CLASS_NAME,L"Web directory",
   WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,originalHeight,
   std::max(660L,client.right),340,target,nullptr,moduleHandle,nullptr);
 if(!child){DeleteObject(font);font=nullptr;return;}
 SetTimer(child,1,500,nullptr);
 SetWindowPos(child,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
 adjustingFrame=true;
 if(reservedHeight)SetWindowPos(target,nullptr,0,0,window.right-window.left,
   window.bottom-window.top+reservedHeight,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
 adjustingFrame=false;
 if(!reservedHeight)ShowWindow(child,SW_HIDE);
 RECT resized;GetClientRect(target,&resized);
 MoveWindow(child,0,originalHeight,resized.right,std::max(180L,resized.bottom-originalHeight),TRUE);
 AcquireSRWLockExclusive(&pendingLock);auto initial=std::move(pendingURL);
 pendingURL.clear();ReleaseSRWLockExclusive(&pendingLock);
 if(initial.empty())initial=L"https://3.onj.me/phonetones/unzipped/Samsung/SGH-D500/";
 Navigate(initial);
 #ifndef MIDRADIO_DISABLE_SKIN
 StartEnglishSkinLabels(moduleHandle);
 #endif
}
DWORD WINAPI UIThread(void*){
 HWND target=nullptr;
 for(int i=0;i<120&&!target;++i){EnumWindows(FindMain,reinterpret_cast<LPARAM>(&target));if(!target)Sleep(500);}
 if(!target)return 0;
 Sleep(1000);if(!IsWindow(target))return 0;
 mainWindow=target;
 initializeHook=SetWindowsHookExW(WH_CALLWNDPROC,InitializeOnGUIThread,moduleHandle,GetWindowThreadProcessId(target,nullptr));
 if(!initializeHook){runtimeLog("GUI initialization hook failed: %lu",GetLastError());return 0;}
 PostMessageW(target,WM_INITIALIZE,0,0);
 for(int i=0;i<100&&!pane&&IsWindow(target);++i)Sleep(50);
 UnhookWindowsHookEx(initializeHook);initializeHook=nullptr;
 return 0;
}
}
void StartDirectoryUI(HMODULE mod){if(started.exchange(true))return;moduleHandle=mod;HANDLE h=CreateThread(nullptr,0,UIThread,nullptr,0,nullptr);if(h)CloseHandle(h);else started=false;}
void NavigateDirectory(const std::wstring& url){HWND h=pane;if(h){auto p=new std::wstring(url);if(!PostMessageW(h,WM_NAVIGATE,0,reinterpret_cast<LPARAM>(p)))delete p;}else{AcquireSRWLockExclusive(&pendingLock);pendingURL=url;ReleaseSRWLockExclusive(&pendingLock);}}
void SetDirectoryStatus(const std::wstring& text){HWND h=pane;if(!h)return;auto p=new std::wstring(text);if(!PostMessageW(h,WM_STATUS,0,reinterpret_cast<LPARAM>(p)))delete p;}








