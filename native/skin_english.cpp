#define WIN32_LEAN_AND_MEAN
#include "skin_english.h"
#include <cstring>
#include "runtime_log.h"

namespace {
using Blit = BOOL(WINAPI*)(HDC,int,int,int,int,HDC,int,int,DWORD);
Blit originalBlit=nullptr;
LONG started=0;
LONG blitCount=0,renderCount=0;
CRITICAL_SECTION renderLock;
HFONT normalFont=nullptr,smallFont=nullptr,detailsFont=nullptr;
constexpr int DetailsWidth=61;
constexpr int DetailsPadding=9; // Six pixels farther right than 0.2.0's 3px inset.
struct Sprite {int id,width,height;HBITMAP bitmap;HDC dc;HGDIOBJ old;};
Sprite sprites[]={{430,112,20,nullptr,nullptr,nullptr},
                  {359,60,52,nullptr,nullptr,nullptr},
                  {368,47,378,nullptr,nullptr,nullptr}};

bool Matches(HDC source,const Sprite& s){
 // Resource identity is verified before modifying a bitmap of the same size.
 // Sampling includes each sprite state's text and icon, plus background edges.
 for(int row=0;row<13;++row){
  int y=(s.height-1)*row/12;
  for(int col=0;col<9;++col){int x=(s.width-1)*col/8;
   if(GetPixel(source,x,y)!=GetPixel(s.dc,x,y))return false;
  }
 }
 return true;
}
void Label(HDC dc,RECT rect,const wchar_t* text,COLORREF foreground,COLORREF background,HFONT font){
 HBRUSH brush=CreateSolidBrush(background);FillRect(dc,&rect,brush);DeleteObject(brush);
 SelectObject(dc,font);SetTextColor(dc,foreground);SetBkMode(dc,TRANSPARENT);
 SetTextAlign(dc,TA_LEFT|TA_TOP|TA_NOUPDATECP);SetTextCharacterExtra(dc,0);
 SIZE extent={};int length=lstrlenW(text);GetTextExtentPoint32W(dc,text,length,&extent);
 if(length>1&&extent.cx>rect.right-rect.left)SetTextCharacterExtra(dc,-((extent.cx-(rect.right-rect.left)+length-2)/(length-1)));
 DrawTextW(dc,text,-1,&rect,DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX);
}
void DetailsLabel(HDC dc,RECT rect,COLORREF background){
 int saved=SaveDC(dc);
 HBRUSH brush=CreateSolidBrush(background);FillRect(dc,&rect,brush);DeleteObject(brush);
 rect.left+=DetailsPadding;
 SelectObject(dc,detailsFont);SetTextColor(dc,RGB(2,107,174));SetBkMode(dc,TRANSPARENT);
 SetTextAlign(dc,TA_LEFT|TA_TOP|TA_NOUPDATECP);SetTextCharacterExtra(dc,0);
 DrawTextW(dc,L"Track Details",-1,&rect,DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_NOPREFIX);
 RestoreDC(dc,saved);
}
void Render(HDC dc,const Sprite& sprite){
 InterlockedIncrement(&renderCount);runtimeLog("skin render sprite=%d",sprite.id);
 int saved=SaveDC(dc);SetMapMode(dc,MM_TEXT);SetWindowOrgEx(dc,0,0,nullptr);SetViewportOrgEx(dc,0,0,nullptr);
 if(sprite.id==430){
  RECT area={0,0,112,20};
  Label(dc,area,L"Select Screen",RGB(78,78,78),GetPixel(sprite.dc,111,0),normalFont);
 }else if(sprite.id==359){
  for(int state=0;state<4;++state){
   // Preserve the original circular arrow icon and its state.
   RECT area={14,state*13,60,state*13+13};
   // Clear the old sprite lettering; the wider click-through label draws it
   // once, without compressed/duplicate glyphs or touching the arrow icon.
   HBRUSH brush=CreateSolidBrush(GetPixel(sprite.dc,59,state*13+6));
   FillRect(dc,&area,brush);DeleteObject(brush);
  }
 }else if(sprite.id==368){
  static const wchar_t* labels[]={L"Repeat off",L"Repeat all",L"Repeat one"};
  for(int state=0;state<9;++state){
   // Nine 42px states: off/all/one, each normal/pressed/highlighted.
   // The original colored repeat arrows and button borders remain intact.
   RECT area={0,state*42+30,47,state*42+42};
   Label(dc,area,labels[state/3],RGB(98,98,98),
         GetPixel(sprite.dc,46,state*42+35),smallFont);
  }
 }
 RestoreDC(dc,saved);
 OutputDebugStringW(L"MidRadio: English skin labels rendered into native sprite.\n");
}
BOOL WINAPI EnglishBitBlt(HDC destination,int x,int y,int width,int height,HDC source,int sx,int sy,DWORD operation){
 LONG call=InterlockedIncrement(&blitCount);
 if(source){
  HGDIOBJ selected=GetCurrentObject(source,OBJ_BITMAP);BITMAP info={};
  if(selected&&GetObjectW(selected,sizeof(info),&info)==sizeof(info)){
   if(call<=20)runtimeLog("skin blit src=%ldx%ld copy=%dx%d at=%d,%d source=%d,%d",info.bmWidth,info.bmHeight,width,height,x,y,sx,sy);
   for(auto& sprite:sprites){
    if(info.bmWidth!=sprite.width||info.bmHeight!=sprite.height||selected==sprite.bitmap)continue;
    EnterCriticalSection(&renderLock);
    if(Matches(source,sprite))Render(source,sprite);
    LeaveCriticalSection(&renderLock);
    break;
   }
  }
 }
 return originalBlit(destination,x,y,width,height,source,sx,sy,operation);
}
bool HookMainImport(){
 auto base=reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr));
 auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
 if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return false;
 auto nt=reinterpret_cast<IMAGE_NT_HEADERS*>(base+dos->e_lfanew);
 if(nt->Signature!=IMAGE_NT_SIGNATURE)return false;
 DWORD rva=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
 if(!rva)return false;
 auto imports=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+rva);
 for(;imports->Name;++imports){
  if(_stricmp(reinterpret_cast<char*>(base+imports->Name),"GDI32.dll")!=0||!imports->OriginalFirstThunk)continue;
  auto names=reinterpret_cast<IMAGE_THUNK_DATA*>(base+imports->OriginalFirstThunk);
  auto slots=reinterpret_cast<IMAGE_THUNK_DATA*>(base+imports->FirstThunk);
  for(;names->u1.AddressOfData;++names,++slots){
   if(IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal))continue;
   auto name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData);
   if(std::strcmp(reinterpret_cast<char*>(name->Name),"BitBlt")!=0)continue;
   DWORD previous=0;
   if(!VirtualProtect(&slots->u1.Function,sizeof(slots->u1.Function),PAGE_READWRITE,&previous))return false;
   originalBlit=reinterpret_cast<Blit>(slots->u1.Function);
   InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&slots->u1.Function),reinterpret_cast<PVOID>(&EnglishBitBlt));
   DWORD ignored=0;VirtualProtect(&slots->u1.Function,sizeof(slots->u1.Function),previous,&ignored);
   return true;
  }
 }
 return false;
}
BOOL CALLBACK Refresh(HWND hwnd,LPARAM){DWORD pid=0;GetWindowThreadProcessId(hwnd,&pid);if(pid==GetCurrentProcessId())RedrawWindow(hwnd,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);return TRUE;}
HWND skinParent=nullptr;
HWND overlays[3]={};
int repeatGroup=0;
int ReadRepeatGroup(){
 if(!skinParent)return repeatGroup;
 HDC dc=GetDC(skinParent);if(!dc)return repeatGroup;
 unsigned long long best=~0ULL;int bestState=0;
 for(int state=0;state<9;++state){
  unsigned long long error=0;int samples=0;
  for(int y=6;y<27;y+=3)for(int x=9;x<37;x+=3){
   COLORREF actual=GetPixel(dc,131+x,49+y),expected=GetPixel(sprites[2].dc,x,state*42+y);
   if(actual==CLR_INVALID||expected==CLR_INVALID)continue;
   int dr=int(GetRValue(actual))-GetRValue(expected),dg=int(GetGValue(actual))-GetGValue(expected),db=int(GetBValue(actual))-GetBValue(expected);
   error+=dr*dr+dg*dg+db*db;++samples;
  }
  if(samples&&error<best){best=error;bestState=state;}
 }
 ReleaseDC(skinParent,dc);
 // Preserve the last known state when covered, minimized, or in another skin.
 if(best<1200000ULL)repeatGroup=bestState/3;
 return repeatGroup;
}
LRESULT CALLBACK OverlayProc(HWND h,UINT message,WPARAM wp,LPARAM lp){
 int index=int(GetWindowLongPtrW(h,GWLP_USERDATA));
 if(message==WM_NCHITTEST)return HTTRANSPARENT;
 if(message==WM_ERASEBKGND)return 1;
 if(message==WM_TIMER){
  RECT parent;GetClientRect(skinParent,&parent);
  int x=index==0?parent.right-128:index==1?parent.right-296:131;
  int y=index==0?45:index==1?60:79;
  int width=index==0?112:index==1?DetailsWidth:47;
  int height=index==0?20:index==1?13:12;
  SetWindowPos(h,HWND_TOP,x,y,width,height,SWP_NOACTIVATE);
  if(index==2)ReadRepeatGroup();
  InvalidateRect(h,nullptr,FALSE);return 0;
 }
 if(message==WM_PAINT){
  PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT rect;GetClientRect(h,&rect);
  if(index==0)Label(dc,rect,L"Select Screen",RGB(78,78,78),GetPixel(sprites[0].dc,111,0),normalFont);
  else if(index==1){
   DetailsLabel(dc,rect,GetPixel(sprites[1].dc,59,6));
  }
  else {static const wchar_t* names[]={L"Repeat off",L"Repeat all",L"Repeat one"};Label(dc,rect,names[repeatGroup],RGB(98,98,98),GetPixel(sprites[2].dc,46,35),smallFont);}
  EndPaint(h,&ps);return 0;
 }
 if(message==WM_DESTROY){KillTimer(h,1);return 0;}
 return DefWindowProcW(h,message,wp,lp);
}
BOOL CALLBACK FindSkinParent(HWND h,LPARAM){
 DWORD pid=0;GetWindowThreadProcessId(h,&pid);
 if(pid==GetCurrentProcessId()&&IsWindowVisible(h)&&!GetWindow(h,GW_OWNER)){
  wchar_t title[256]={};GetWindowTextW(h,title,256);
  if(wcsstr(title,L"MidRadio")){skinParent=h;return FALSE;}
 }
 return TRUE;
}
void InstallLabelOverlays(){
 EnumWindows(FindSkinParent,0);if(!skinParent){runtimeLog("skin overlay: main window missing");return;}
 WNDCLASSW cls={};cls.lpfnWndProc=OverlayProc;cls.hInstance=GetModuleHandleW(nullptr);
 cls.lpszClassName=L"MidRadioEnglishSkinLabel";RegisterClassW(&cls);
 // These are the standard skin's INIFILE coordinates, including its right-
 // anchored elements. HTTRANSPARENT keeps all clicks on the original controls.
 for(int i=0;i<3;++i){
  HWND h=CreateWindowExW(WS_EX_TRANSPARENT|WS_EX_NOACTIVATE,cls.lpszClassName,L"",
      WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,0,0,1,1,skinParent,nullptr,cls.hInstance,nullptr);
  if(!h)continue;
  overlays[i]=h;SetWindowLongPtrW(h,GWLP_USERDATA,i);
  SendMessageW(h,WM_TIMER,1,0);SetTimer(h,1,200,nullptr);
 }
 runtimeLog("skin click-through overlays installed parent=%p",skinParent);
}
}
void StartEnglishSkinLabels(HMODULE extensionModule){
 (void)extensionModule;
 if(InterlockedCompareExchange(&started,1,0)!=0)return;
 InitializeCriticalSection(&renderLock);
 normalFont=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                      OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Tahoma");
 smallFont=CreateFontW(-9,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                     OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Tahoma");
 detailsFont=CreateFontW(-8,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                     OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Arial");
 bool ready=normalFont&&smallFont&&detailsFont;
 for(auto& sprite:sprites){
  sprite.bitmap=LoadBitmapW(GetModuleHandleW(nullptr),MAKEINTRESOURCEW(sprite.id));
  sprite.dc=CreateCompatibleDC(nullptr);
  if(!sprite.bitmap||!sprite.dc){ready=false;continue;}
  sprite.old=SelectObject(sprite.dc,sprite.bitmap);
  BITMAP info={};GetObjectW(sprite.bitmap,sizeof(info),&info);
  if(info.bmWidth!=sprite.width||info.bmHeight!=sprite.height)ready=false;
 }
 runtimeLog("skin references ready=%d",ready);
 if(ready){bool hooked=HookMainImport();runtimeLog("skin BitBlt hook installed=%d",hooked);EnumWindows(Refresh,0);InstallLabelOverlays();}
 else OutputDebugStringW(L"MidRadio: English skin hook could not be installed for this build.\n");
 // The extension is pinned for process lifetime; fonts/reference DCs must also
 // remain valid for skin changes and later repaint calls.
}



