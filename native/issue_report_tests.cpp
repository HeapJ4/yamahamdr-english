#include "issue_report.h"
#include <cassert>
#include <iostream>
int main(){
 using namespace IssueReport;
 assert(Trim(L" \r\n\t ").empty());assert(Trim(L" title \r\n")==L"title");
 assert(Encode(L"a &b?x=1#c\r\n")=="a%20%26b%3Fx%3D1%23c%0D%0A");
 assert(Encode(L"\u042f")=="%D0%AF");assert(Encode(L"\U0001f600")=="%F0%9F%98%80");
 auto url=URL(L"test & title",L"body ? & #");
 assert(url=="https://github.com/HeapJ4/yamahamdr-english/issues/new?title=test%20%26%20title&body=body%20%3F%20%26%20%23");
 assert(Body(L"body",false,L"private test")==L"body");assert(Body(L"body",true,L"version").find(L"version")!=std::wstring::npos);
 assert(URL(L"title",std::wstring(16000,L'\u042f')).size()>1900);
 std::wstring malformed(1,wchar_t(0xd800));bool rejected=false;try{Encode(malformed);}catch(...){rejected=true;}assert(rejected);
 std::cout<<"Issue draft encoding, optional diagnostics, Unicode and long-report detection passed.\n";
}
