// Focused parser and URL tests. This test translation unit includes the implementation.
#include "directory_ui.cpp"
#include <cassert>
#include <iostream>
bool OpenTrackURL(const std::wstring&,const std::wstring&){return true;}
bool OpenTrackPlaylist(const std::vector<std::pair<std::wstring,std::wstring>>&,const std::wstring&){return true;}
int main(){
 assert(DecodeEntities(L"A &amp; B &#x1f3b5;")==L"A & B \U0001f3b5");
 assert(DecodeEntities(L"&unknown; &#garbage;")==L"&unknown; &#garbage;");
 assert(PercentDecode(L"%E6%97%A5%E6%9C%AC%20file.mmf")==L"日本 file.mmf");
 assert(Resolve(L"https://example.org/music/",L"../folder/song%20one.mp3?x=1&y=2")==L"https://example.org/folder/song%20one.mp3?x=1&y=2");
 assert(Resolve(L"https://example.org/music/",L"javascript:alert(1)").empty());
 assert(Supported(L"https://example.org/song%2Emp3?x=1"));
 assert(!Supported(L"https://example.org/song.mp3.exe"));
 for(auto extension:{L".mmf",L".mrm",L".mrl",L".wav",L".mp3",L".ogg",L".m4a"})assert(Supported(std::wstring(L"https://example.org/song")+extension));
 auto r=Parse(L"<html><a href='../'>Parent Directory</a><a HREF=\"folder/\">folder/</a><a href='a%20b.MMF?x=1&amp;y=2'>a &amp; b.MMF</a><a href='a%20b.MMF?x=1&amp;y=2'>duplicate</a><a href='c.ogg'>c.ogg</a><a href='x.txt'>x</a><a href='?C=N;O=D'>sort</a><a href='javascript:x'>bad</a></html>",L"https://example.org/music/");
 assert(r.size()==4);assert(r[0].directory&&r[1].directory);
 assert(r[2].url==L"https://example.org/music/a%20b.MMF?x=1&y=2");assert(r[2].title==L"a & b.MMF");
 auto base=Parse(L"<base href='/audio/'><a href='x.wav'>x.wav</a>",L"https://example.org/music/");
 assert(base.size()==1&&base[0].url==L"https://example.org/audio/x.wav");
 auto malformed=Parse(L"<a href='x.mp3' title='a > b'>x.mp3</a><a href='unterminated",L"http://localhost/");assert(malformed.size()==1);
 std::cout<<"Directory parsing and URL resolution tests passed.\n";
}
