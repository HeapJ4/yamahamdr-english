#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <stdexcept>
#include "issue_report.h"
#include "app_version.h"
namespace {
constexpr int Title=101,Description=102,Write=103,Preview=104,Diagnostics=105,Copy=106,Continue=107,Cancel=108,Bold=109,Italic=110,Code=111;
COLORREF background=RGB(13,17,23),field=RGB(22,27,34),ink=RGB(230,237,243);
HBRUSH dark=nullptr,editBrush=nullptr;HFONT font=nullptr,heading=nullptr;
HWND titleBox=nullptr,descriptionBox=nullptr,previewBox=nullptr;bool showingPreview=false;
std::wstring Text(HWND h){int n=GetWindowTextLengthW(h);std::wstring s(n+1,L'\0');GetWindowTextW(h,&s[0],n+1);s.resize(n);return s;}
std::wstring Basic(){
 OSVERSIONINFOW info={};info.dwOSVersionInfoSize=sizeof(info);
 using VersionFn=LONG(WINAPI*)(OSVERSIONINFOW*);
 auto fn=reinterpret_cast<VersionFn>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"RtlGetVersion"));
 BOOL wow=FALSE;IsWow64Process(GetCurrentProcess(),&wow);
 std::wstring result=L"MidRadio English + Directory ";result+=MIDRADIO_VERSION;
 if(fn&&fn(&info)==0)result+=L"\r\nWindows "+std::to_wstring(info.dwMajorVersion)+L"."+std::to_wstring(info.dwMinorVersion)+L" build "+std::to_wstring(info.dwBuildNumber);
 result+=wow?L"\r\nx86 app on 64-bit Windows":L"\r\nx86 app";
 return result+L"\r\nNo logs, paths, URLs, filenames or usernames collected.";
}
bool CopyText(HWND owner,const std::wstring& text){
 if(!OpenClipboard(owner))return false;
 HGLOBAL buffer=GlobalAlloc(GMEM_MOVEABLE,(text.size()+1)*sizeof(wchar_t));if(!buffer){CloseClipboard();return false;}
 void* bytes=GlobalLock(buffer);if(!bytes){GlobalFree(buffer);CloseClipboard();return false;}
 memcpy(bytes,text.c_str(),(text.size()+1)*sizeof(wchar_t));GlobalUnlock(buffer);
 bool ok=EmptyClipboard()&&SetClipboardData(CF_UNICODETEXT,buffer);if(!ok)GlobalFree(buffer);CloseClipboard();return ok;
}
std::wstring Report(HWND h){return IssueReport::Body(Text(descriptionBox),IsDlgButtonChecked(h,Diagnostics)==BST_CHECKED,Basic());}
void UpdatePreview(HWND h){SetWindowTextW(previewBox,(L"# "+Text(titleBox)+L"\r\n\r\n"+Report(h)).c_str());}
void Layout(HWND h){
 RECT r;GetClientRect(h,&r);int w=r.right,b=r.bottom;
 MoveWindow(GetDlgItem(h,201),24,18,w-48,30,TRUE);MoveWindow(titleBox,24,54,w-48,34,TRUE);
 MoveWindow(GetDlgItem(h,202),24,106,w-48,26,TRUE);
 MoveWindow(GetDlgItem(h,Write),24,138,90,30,TRUE);MoveWindow(GetDlgItem(h,Preview),120,138,100,30,TRUE);
 MoveWindow(GetDlgItem(h,Bold),w-176,138,46,30,TRUE);MoveWindow(GetDlgItem(h,Italic),w-124,138,46,30,TRUE);MoveWindow(GetDlgItem(h,Code),w-72,138,48,30,TRUE);
 MoveWindow(descriptionBox,24,178,w-48,b-340,TRUE);MoveWindow(previewBox,24,178,w-48,b-340,TRUE);
 MoveWindow(GetDlgItem(h,Diagnostics),24,b-148,w-48,25,TRUE);
 MoveWindow(GetDlgItem(h,203),24,b-117,w-48,38,TRUE);
 MoveWindow(GetDlgItem(h,Copy),24,b-58,136,32,TRUE);MoveWindow(GetDlgItem(h,Cancel),w-320,b-58,90,32,TRUE);MoveWindow(GetDlgItem(h,Continue),w-220,b-58,196,32,TRUE);
}
void Wrap(const wchar_t* marker){if(showingPreview)return;DWORD a=0,b=0;SendMessageW(descriptionBox,EM_GETSEL,WPARAM(&a),LPARAM(&b));auto text=Text(descriptionBox);if(b>text.size())return;auto insert=std::wstring(marker)+text.substr(a,b-a)+marker;SendMessageW(descriptionBox,EM_REPLACESEL,TRUE,LPARAM(insert.c_str()));SetFocus(descriptionBox);}
LRESULT CALLBACK Window(HWND h,UINT m,WPARAM w,LPARAM l){
 try{
 switch(m){
 case WM_CREATE:{
  auto add=[&](const wchar_t* cls,const wchar_t* text,DWORD style,int id){HWND c=CreateWindowExW(wcscmp(cls,L"EDIT")==0?WS_EX_CLIENTEDGE:0,cls,text,WS_VISIBLE|WS_CHILD|style,0,0,1,1,h,HMENU(INT_PTR(id)),GetModuleHandleW(nullptr),nullptr);SendMessageW(c,WM_SETFONT,WPARAM(font),TRUE);return c;};
  HWND label=add(L"STATIC",L"Add a title *",0,201);SendMessageW(label,WM_SETFONT,WPARAM(heading),TRUE);
  titleBox=add(L"EDIT",L"",WS_TABSTOP|ES_AUTOHSCROLL,Title);SendMessageW(titleBox,EM_SETLIMITTEXT,200,0);
  label=add(L"STATIC",L"Add a description",0,202);SendMessageW(label,WM_SETFONT,WPARAM(heading),TRUE);
  add(L"BUTTON",L"Write",WS_TABSTOP,Write);add(L"BUTTON",L"Preview",WS_TABSTOP,Preview);add(L"BUTTON",L"B",WS_TABSTOP,Bold);add(L"BUTTON",L"I",WS_TABSTOP,Italic);add(L"BUTTON",L"< >",WS_TABSTOP,Code);
  descriptionBox=add(L"EDIT",L"### What happened?\r\n\r\n\r\n### Steps to reproduce\r\n1. \r\n\r\n### Expected behavior\r\n",WS_TABSTOP|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN|WS_VSCROLL,Description);SendMessageW(descriptionBox,EM_SETLIMITTEXT,16000,0);
  previewBox=add(L"EDIT",L"",ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL,204);ShowWindow(previewBox,SW_HIDE);
  add(L"BUTTON",L"Include basic diagnostics (version + Windows build only)",WS_TABSTOP|BS_AUTOCHECKBOX,Diagnostics);
  add(L"STATIC",L"Review on GitHub before creating the issue. No logs or files are sent automatically.\r\nAttach screenshots or reviewed logs on GitHub after opening the form.",0,203);
  add(L"BUTTON",L"Copy report",WS_TABSTOP,Copy);add(L"BUTTON",L"Cancel",WS_TABSTOP,Cancel);add(L"BUTTON",L"Continue on GitHub",WS_TABSTOP|BS_OWNERDRAW,Continue);Layout(h);SetFocus(titleBox);return 0;
 }
 case WM_SIZE:Layout(h);return 0;
 case WM_GETMINMAXINFO:{auto p=reinterpret_cast<MINMAXINFO*>(l);p->ptMinTrackSize={680,520};return 0;}
 case WM_CTLCOLORSTATIC:case WM_CTLCOLORBTN:case WM_CTLCOLOREDIT:{HDC dc=reinterpret_cast<HDC>(w);SetTextColor(dc,ink);SetBkColor(dc,m==WM_CTLCOLOREDIT?field:background);return LRESULT(m==WM_CTLCOLOREDIT?editBrush:dark);}
 case WM_DRAWITEM:{auto item=reinterpret_cast<DRAWITEMSTRUCT*>(l);if(item->CtlID==Continue){HBRUSH green=CreateSolidBrush(item->itemState&ODS_SELECTED?RGB(26,112,48):RGB(35,134,54));FillRect(item->hDC,&item->rcItem,green);DeleteObject(green);SetBkMode(item->hDC,TRANSPARENT);SetTextColor(item->hDC,RGB(255,255,255));SelectObject(item->hDC,font);DrawTextW(item->hDC,L"Continue on GitHub",-1,&item->rcItem,DT_CENTER|DT_VCENTER|DT_SINGLELINE);if(item->itemState&ODS_FOCUS)DrawFocusRect(item->hDC,&item->rcItem);return TRUE;}break;}
 case WM_COMMAND:{int id=LOWORD(w);
  if(id==Write||id==Preview){showingPreview=id==Preview;UpdatePreview(h);ShowWindow(descriptionBox,showingPreview?SW_HIDE:SW_SHOW);ShowWindow(previewBox,showingPreview?SW_SHOW:SW_HIDE);return 0;}
  if(id==Diagnostics||((id==Title||id==Description)&&HIWORD(w)==EN_CHANGE)){if(showingPreview)UpdatePreview(h);return 0;}
  if(id==Bold||id==Italic||id==Code){Wrap(id==Bold?L"**":id==Italic?L"*":L"`");return 0;}
  if(id==Cancel){DestroyWindow(h);return 0;}
  if(id==Copy){if(!CopyText(h,L"# "+Text(titleBox)+L"\r\n\r\n"+Report(h)))MessageBoxW(h,L"Clipboard is busy. Try again.",L"Report issue",MB_OK|MB_ICONERROR);return 0;}
  if(id==Continue){auto title=IssueReport::Trim(Text(titleBox));if(title.empty()){MessageBoxW(h,L"Add a title first.",L"Report issue",MB_OK);SetFocus(titleBox);return 0;}
   auto url=IssueReport::URL(title,Report(h));bool copied=false;
   if(url.size()>1900){if(MessageBoxW(h,L"This report is long. Copy its description and open GitHub, then paste it into the issue form?",L"Report issue",MB_YESNO|MB_ICONQUESTION)!=IDYES)return 0;if(!CopyText(h,Report(h))){MessageBoxW(h,L"Clipboard is busy. Nothing was opened; try again.",L"Report issue",MB_OK|MB_ICONERROR);return 0;}copied=true;url=IssueReport::URL(title,L"");}
   std::wstring wide(url.begin(),url.end());auto result=ShellExecuteW(h,L"open",wide.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
   if(INT_PTR(result)<=32)MessageBoxW(h,L"Could not open the browser. Use Copy report and visit github.com/HeapJ4/yamahamdr-english/issues.",L"Report issue",MB_OK|MB_ICONERROR);
   else if(copied)MessageBoxW(h,L"Paste the copied description on GitHub, review it, then click Create.",L"Report copied",MB_OK);
   return 0;
  }break;
 }
 case WM_DESTROY:PostQuitMessage(0);return 0;
 }
 }catch(...){MessageBoxW(h,L"The report could not be prepared. Copy your text and try again; no issue was submitted.",L"Report issue",MB_OK|MB_ICONERROR);}
 return DefWindowProcW(h,m,w,l);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR arguments,int show){
 dark=CreateSolidBrush(background);editBrush=CreateSolidBrush(field);
 font=CreateFontW(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");heading=CreateFontW(-20,0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
 WNDCLASSW cls={};cls.lpfnWndProc=Window;cls.hInstance=instance;cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hbrBackground=dark;cls.lpszClassName=L"MidRadioIssueReport";
 if(!RegisterClassW(&cls))return 1;
 HWND window=CreateWindowW(cls.lpszClassName,L"Report an issue - MidRadio",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,900,700,nullptr,nullptr,instance,nullptr);if(!window)return 1;
 if(arguments&&wcscmp(arguments,L"--self-test")==0){
  // Exercise our own hidden form, never the user's player, browser or clipboard.
  if(!titleBox||!descriptionBox||!previewBox||!GetDlgItem(window,Continue))return 2;
  SetWindowTextW(titleBox,L"Startup regression test");
  SetWindowTextW(descriptionBox,L"Report preview test");
  for(int i=0;i<100;++i){SendMessageW(window,WM_COMMAND,Preview,0);if(Text(previewBox).find(L"Report preview test")==std::wstring::npos)return 3;SendMessageW(window,WM_COMMAND,Write,0);}
  SendMessageW(GetDlgItem(window,Diagnostics),BM_SETCHECK,BST_CHECKED,0);
  SendMessageW(window,WM_COMMAND,Preview,0);
  if(Text(previewBox).find(MIDRADIO_VERSION)==std::wstring::npos)return 4;
  DestroyWindow(window);DeleteObject(font);DeleteObject(heading);DeleteObject(dark);DeleteObject(editBrush);return 0;
 }
 ShowWindow(window,show);MSG m;int result;while((result=GetMessageW(&m,nullptr,0,0))>0){if(!IsDialogMessageW(window,&m)){TranslateMessage(&m);DispatchMessageW(&m);}}
 DeleteObject(font);DeleteObject(heading);DeleteObject(dark);DeleteObject(editBrush);return result==-1?1:0;
}
