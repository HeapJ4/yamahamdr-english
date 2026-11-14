#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <stdexcept>
namespace IssueReport {
inline std::wstring Trim(std::wstring value){auto b=value.find_first_not_of(L" \r\n\t");return b==value.npos?L"":value.substr(b,value.find_last_not_of(L" \r\n\t")-b+1);}
inline std::string Encode(const std::wstring& value){
 if(value.empty())return "";
 int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value.data(),int(value.size()),nullptr,0,nullptr,nullptr);
 if(!n)throw std::runtime_error("Invalid Unicode text");
 std::string bytes(n,'\0');WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value.data(),int(value.size()),&bytes[0],n,nullptr,nullptr);
 const char* hex="0123456789ABCDEF";std::string out;
 for(unsigned char c:bytes){if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~')out+=char(c);else{out+='%';out+=hex[c>>4];out+=hex[c&15];}}
 return out;
}
inline std::string URL(const std::wstring& title,const std::wstring& body){return "https://github.com/HeapJ4/yamahamdr-english/issues/new?title="+Encode(title)+"&body="+Encode(body);}
inline std::wstring Body(const std::wstring& text,bool diagnostics,const std::wstring& basic){return text+(diagnostics?L"\r\n\r\n### Basic diagnostics\r\n"+basic:L"");}
}
