#pragma once
#include <windows.h>
#include <cstdio>
#include <cstdarg>
static void runtimeLog(const char* format,...) {
    char text[2048];va_list args;va_start(args,format);vsnprintf(text,sizeof(text),format,args);va_end(args);
    wchar_t path[32768];GetModuleFileNameW(nullptr,path,32768);wchar_t* p=wcsrchr(path,L'\\');if(!p)return;wcscpy(p+1,L"runtime.log");
    HANDLE h=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(h!=INVALID_HANDLE_VALUE){DWORD n;WriteFile(h,text,(DWORD)strlen(text),&n,nullptr);WriteFile(h,"\r\n",2,&n,nullptr);CloseHandle(h);}
}
static LONG WINAPI runtimeCrash(EXCEPTION_POINTERS* info) {
    MEMORY_BASIC_INFORMATION m={};VirtualQuery((void*)info->ContextRecord->Eip,&m,sizeof(m));
    wchar_t path[32768]={};GetModuleFileNameW((HMODULE)m.AllocationBase,path,32768);
    char module[1024];WideCharToMultiByte(CP_UTF8,0,path,-1,module,sizeof(module),nullptr,nullptr);
    runtimeLog("CRASH code=%08lx ip=%08lx module=%s offset=%08lx eax=%08lx ecx=%08lx edx=%08lx esp=%08lx",info->ExceptionRecord->ExceptionCode,info->ContextRecord->Eip,module,(DWORD)(info->ContextRecord->Eip-(DWORD)m.AllocationBase),info->ContextRecord->Eax,info->ContextRecord->Ecx,info->ContextRecord->Edx,info->ContextRecord->Esp);
    VirtualQuery((void*)info->ContextRecord->Esp,&m,sizeof(m));
    if(m.State==MEM_COMMIT&&!(m.Protect&(PAGE_NOACCESS|PAGE_GUARD))&&info->ContextRecord->Esp+128<=(DWORD)m.BaseAddress+m.RegionSize){
        DWORD* stack=(DWORD*)info->ContextRecord->Esp;
        for(int i=0;i<32;i++){MEMORY_BASIC_INFORMATION a={};VirtualQuery((void*)stack[i],&a,sizeof(a));wchar_t mod[MAX_PATH]={};GetModuleFileNameW((HMODULE)a.AllocationBase,mod,MAX_PATH);char name[1024]={};WideCharToMultiByte(CP_UTF8,0,mod,-1,name,sizeof(name),nullptr,nullptr);runtimeLog("stack[%d]=%08lx module=%s offset=%08lx",i,stack[i],name,stack[i]-(DWORD)a.AllocationBase);}
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
