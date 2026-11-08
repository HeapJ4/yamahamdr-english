#include "media_prepare.h"
#include <windows.h>
#include <cstdio>
#include <string>

int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;
    for(auto extension:{L"ogg",L"m4a",L"mp3",L"wav",L"mmf"}){
        if(argc==4&&!wcscmp(argv[3],L"--no-m4a")&&!wcscmp(extension,L"m4a"))continue;
        std::wstring path,error;
        if(!PrepareMedia(std::wstring(argv[2])+L"/tone."+extension,argv[1],path,error)){
            fwprintf(stderr,L"%ls: %ls\n",extension,error.c_str());return 1;
        }
        FILE* file=_wfopen(path.c_str(),L"rb");if(!file)return 1;
        char header[4]={};fread(header,1,4,file);fclose(file);
        if((!wcscmp(extension,L"ogg")||!wcscmp(extension,L"m4a")||!wcscmp(extension,L"wav"))&&memcmp(header,"RIFF",4))return 1;
        if(!wcscmp(extension,L"mmf")&&memcmp(header,"MMMD",4))return 1;
        fwprintf(stdout,L"Prepared %ls: %ls\n",extension,path.c_str());
    }
    std::wstring encoded,error;
    if(!PrepareMedia(std::wstring(argv[2])+L"/tone%2Eogg",argv[1],encoded,error))return 1;
    FILE* damaged=_wfopen(encoded.c_str(),L"wb");if(!damaged)return 1;
    fputs("interrupted PCM cache",damaged);fclose(damaged);
    if(!PrepareMedia(std::wstring(argv[2])+L"/tone%2Eogg",argv[1],encoded,error))return 1;
    FILE* repaired=_wfopen(encoded.c_str(),L"rb");if(!repaired)return 1;
    char signature[4]={};fread(signature,1,4,repaired);fclose(repaired);
    if(memcmp(signature,"RIFF",4))return 1;
    puts("Encoded OGG extension and interrupted PCM cache recovery passed.");
    std::wstring path;
    if(PrepareMedia(std::wstring(argv[2])+L"/broken.ogg",argv[1],path,error))return 1;
    puts("HTTP download, PCM preparation, native-file preservation and malformed OGG rejection passed.");
    return 0;
}
