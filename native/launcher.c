// Maintainable replacement source for the owner's small launcher.
// The release retains native/prebuilt/MidRadio.exe byte-for-byte; this source
// is not claimed to reproduce that binary's exact compiler output.
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <wchar.h>

int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR arguments,int show){
    (void)instance;(void)previous;
    wchar_t folder[32768],target[32768];
    DWORD count=GetModuleFileNameW(NULL,folder,32768);
    if(!count||count>=32768)return 1;
    wchar_t* slash=wcsrchr(folder,L'\\');
    if(!slash)return 1;
    *slash=0;
    if(wcslen(folder)+25>=32768)return 1;
    wcscpy(target,folder);wcscat(target,L"\\MidRadio-Directory.exe");
    HINSTANCE result=ShellExecuteW(NULL,L"open",target,arguments,folder,show);
    if((INT_PTR)result<=32){
        MessageBoxW(NULL,L"Could not open MidRadio-Directory.exe. Extract the entire portable ZIP into one folder.",L"MidRadio",MB_OK|MB_ICONERROR);
        return 1;
    }
    return 0;
}
