// A separate bounded decoder process; no reverse-engineered Yamaha codec ABI.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include "third_party/stb_vorbis.c"

constexpr uint64_t MAX_PCM = 512ULL * 1024 * 1024;
struct WaveWriter {
    FILE* file = nullptr;
    uint32_t bytes = 0, rate = 0;
    uint16_t channels = 0;
    bool begin(const wchar_t* path, uint32_t hz, uint16_t count) {
        rate=hz; channels=count;
        if (!hz || count<1 || count>2) return false;
        file=_wfopen(path,L"wb+");
        if (!file) return false;
        unsigned char header[44]={};
        return fwrite(header,1,sizeof(header),file)==sizeof(header);
    }
    bool write(const void* data, size_t count) {
        if (!file || uint64_t(bytes)+count>MAX_PCM || count% (channels*2)) return false;
        if (fwrite(data,1,count,file)!=count) return false;
        bytes+=uint32_t(count); return true;
    }
    bool finish() {
        if (!file || !bytes) return false;
        unsigned char header[44]={};
        auto u16=[&](int at,uint16_t v){header[at]=v;header[at+1]=v>>8;};
        auto u32=[&](int at,uint32_t v){for(int n=0;n<4;++n)header[at+n]=v>>(n*8);};
        memcpy(header,"RIFF",4);u32(4,36+bytes);memcpy(header+8,"WAVEfmt ",8);
        u32(16,16);u16(20,1);u16(22,channels);u32(24,rate);
        u32(28,rate*channels*2);u16(32,channels*2);u16(34,16);
        memcpy(header+36,"data",4);u32(40,bytes);
        if (fseek(file,0,SEEK_SET) || fwrite(header,1,44,file)!=44) return false;
        bool ok=fflush(file)==0;fclose(file);file=nullptr;return ok;
    }
    ~WaveWriter(){if(file)fclose(file);}
};
template<class T> struct Com {
    T* p=nullptr;
    ~Com(){if(p)p->Release();}
    T** out(){return &p;}
    T* operator->(){return p;}
};

static bool decodeVorbis(const wchar_t* input,const wchar_t* output) {
    FILE* source=_wfopen(input,L"rb");if(!source)return false;
    int error=0;
    stb_vorbis* decoder=stb_vorbis_open_file(source,1,&error,nullptr);
    if(!decoder)return false;
    auto info=stb_vorbis_get_info(decoder);
    WaveWriter wave;
    bool ok=info.channels>=1&&info.channels<=2&&wave.begin(output,info.sample_rate,info.channels);
    short pcm[8192];
    while(ok){
        int frames=stb_vorbis_get_samples_short_interleaved(decoder,info.channels,pcm,8192);
        if(!frames)break;
        ok=wave.write(pcm,size_t(frames)*info.channels*sizeof(short));
    }
    if(stb_vorbis_get_error(decoder)!=VORBIS__no_error)ok=false;
    stb_vorbis_close(decoder);
    return ok&&wave.finish();
}

static bool decodeMediaFoundation(const wchar_t* input,const wchar_t* output) {
    Com<IMFSourceReader> reader;
    if(FAILED(MFCreateSourceReaderFromURL(input,nullptr,reader.out())))return false;
    reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS,FALSE);
    if(FAILED(reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM,TRUE)))return false;
    Com<IMFMediaType> wanted;
    if(FAILED(MFCreateMediaType(wanted.out())))return false;
    wanted->SetGUID(MF_MT_MAJOR_TYPE,MFMediaType_Audio);
    wanted->SetGUID(MF_MT_SUBTYPE,MFAudioFormat_PCM);
    wanted->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,16);
    wanted->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,2);
    wanted->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,44100);
    if(FAILED(reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,nullptr,wanted.p)))return false;
    Com<IMFMediaType> actual;
    if(FAILED(reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,actual.out())))return false;
    UINT32 rate=0,channels=0,bits=0;
    actual->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,&rate);
    actual->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS,&channels);
    actual->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,&bits);
    WaveWriter wave;
    if(bits!=16||!wave.begin(output,rate,uint16_t(channels)))return false;
    for(;;){
        DWORD flags=0;Com<IMFSample> sample;
        HRESULT result=reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM,0,nullptr,&flags,nullptr,sample.out());
        if(FAILED(result)||(flags&MF_SOURCE_READERF_ERROR))return false;
        if(sample.p){
            Com<IMFMediaBuffer> buffer;
            if(FAILED(sample->ConvertToContiguousBuffer(buffer.out())))return false;
            BYTE* data=nullptr;DWORD length=0;
            if(FAILED(buffer->Lock(&data,nullptr,&length)))return false;
            bool ok=wave.write(data,length);buffer->Unlock();
            if(!ok)return false;
        }
        if(flags&MF_SOURCE_READERF_ENDOFSTREAM)break;
    }
    return wave.finish();
}

int wmain(int argc,wchar_t** argv){
    if(argc!=3){fwprintf(stderr,L"Usage: MidRadioDecode.exe input-file output.wav\n");return 2;}
    std::wstring input=argv[1];
    size_t dot=input.find_last_of(L'.');std::wstring extension=dot==input.npos?L"":input.substr(dot);
    for(auto& c:extension)c=towlower(c);
    bool ok=false;
    if(extension==L".ogg")ok=decodeVorbis(argv[1],argv[2]);
    else {
        HRESULT com=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
        if(SUCCEEDED(com)){
            if(SUCCEEDED(MFStartup(MF_VERSION))){ok=decodeMediaFoundation(argv[1],argv[2]);MFShutdown();}
            CoUninitialize();
        }
    }
    if(!ok){DeleteFileW(argv[2]);fwprintf(stderr,L"This file could not be decoded. OGG requires Vorbis; M4A requires a Windows-supported codec.\n");return 1;}
    return 0;
}
