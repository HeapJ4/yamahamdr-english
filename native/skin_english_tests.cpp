#include "skin_english.cpp"
#include <cassert>
#include <iostream>
#include <algorithm>

int wmain(int argc,wchar_t** argv){
 if(argc>2)return 2;
 HMODULE resources=argc==2?LoadLibraryExW(argv[1],nullptr,LOAD_LIBRARY_AS_DATAFILE):nullptr;
 if(argc==2)assert(resources);
 normalFont=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Tahoma");
 smallFont=CreateFontW(-9,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Tahoma");
 detailsFont=CreateFontW(-8,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Arial");
 {
  HDC dc=CreateCompatibleDC(nullptr);BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth=DetailsWidth;info.bmiHeader.biHeight=-13;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
  void* bits=nullptr;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0);assert(bitmap);
  auto previous=SelectObject(dc,bitmap);SelectObject(dc,detailsFont);SIZE text={};
  GetTextExtentPoint32W(dc,L"Track Details",13,&text);
  std::cerr<<"Track Details measured width: "<<text.cx<<"; available: "<<(DetailsWidth-DetailsPadding)<<"\n";
  assert(text.cx<=DetailsWidth-DetailsPadding);
  RECT rect={0,0,DetailsWidth,13};COLORREF background=RGB(220,230,240);
  DetailsLabel(dc,rect,background);
  int first=DetailsWidth,last=-1;
  for(int y=0;y<13;++y)for(int x=0;x<DetailsWidth;++x)if(GetPixel(dc,x,y)!=background){first=std::min(first,x);last=std::max(last,x);}
  assert(first>=DetailsPadding&&last<DetailsWidth-1&&last>first);
  // Original text starts at player.right-296; this must stop before rate x565.
  assert(800-296+DetailsWidth<=565);
  SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
 }
 if(!resources){std::cout<<"Uncompressed Track Details placement and bounds passed.\n";return 0;}
 for(auto& s:sprites){
  s.bitmap=LoadBitmapW(resources,MAKEINTRESOURCEW(s.id));
  assert(s.bitmap);s.dc=CreateCompatibleDC(nullptr);s.old=SelectObject(s.dc,s.bitmap);
  HBITMAP copy=static_cast<HBITMAP>(CopyImage(s.bitmap,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION));
  HDC dc=CreateCompatibleDC(nullptr);auto old=SelectObject(dc,copy);
  assert(Matches(dc,s));
  // The original reference stays untouched; only this runtime copy changes.
  Render(dc,s);
  assert(!Matches(dc,s));
  if(s.id==368){for(int state=0;state<9;++state)assert(GetPixel(dc,22,state*42+15)==GetPixel(s.dc,22,state*42+15));}
  if(s.id==359){for(int state=0;state<4;++state)assert(GetPixel(dc,5,state*13+5)==GetPixel(s.dc,5,state*13+5));}
  SelectObject(dc,old);DeleteDC(dc);DeleteObject(copy);
 }
 std::cout<<"English sprite recognition/rendering and icon preservation passed.\n";
 return 0;
}


