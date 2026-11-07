#include "skin_english.cpp"
#include <cassert>
#include <iostream>

int wmain(int argc,wchar_t** argv){
 if(argc!=2)return 2;
 HMODULE resources=LoadLibraryExW(argv[1],nullptr,LOAD_LIBRARY_AS_DATAFILE);assert(resources);
 normalFont=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Tahoma");
 smallFont=CreateFontW(-9,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Tahoma");
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


