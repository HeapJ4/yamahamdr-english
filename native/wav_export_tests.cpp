#include <string>
static std::wstring lastStatus;
void SetDirectoryStatus(const std::wstring& text){lastStatus=text;}
#include "wav_export.cpp"
#include <cassert>
#include <iostream>

MMRESULT WINAPI ResetOK(HWAVEOUT){return MMSYSERR_NOERROR;}
void CheckFile(const std::wstring& path,const short* samples,size_t count){
 FILE* file=_wfopen(path.c_str(),L"rb");assert(file);
 unsigned char header[44];assert(fread(header,1,44,file)==44);
 assert(!memcmp(header,"RIFF",4)&&!memcmp(header+8,"WAVEfmt ",8)&&!memcmp(header+36,"data",4));
 assert(header[22]==2&&header[24]==0x44&&header[25]==0xac&&header[34]==16);
 assert(header[40]==count*sizeof(short));
 std::vector<short> actual(count);assert(fread(actual.data(),sizeof(short),count,file)==count);fclose(file);
 assert(!memcmp(actual.data(),samples,count*sizeof(short)));
}
int wmain(){
 wchar_t temp[MAX_PATH];assert(GetTempPathW(MAX_PATH,temp));
 std::wstring path=std::wstring(temp)+L"MidRadio-export-test-"+std::to_wstring(GetCurrentProcessId())+L".wav";
 hooksReady=true;HWAVEOUT device=reinterpret_cast<HWAVEOUT>(1);
 WAVEFORMATEX format={WAVE_FORMAT_PCM,2,44100,176400,4,16,0};formats[device]=format;
 short pcm[]={0,0,1000,-1000,3000,-3000,100,-100};std::wstring error;
 assert(BeginWaveExport(path,error));assert(WaveExportActive());
 assert(!BeginWaveExport(path,error));
 CaptureBlock(device,pcm,sizeof(pcm));assert(recordedBytes==0); // Ignore previous track's tail.
 ArmForDevice(device);CaptureBlock(reinterpret_cast<HWAVEOUT>(2),pcm,sizeof(pcm));assert(recordedBytes==0);
 CaptureBlock(device,pcm,sizeof(pcm));assert(recordedBytes==sizeof(pcm));
 FinishWaveExport();assert(!WaveExportActive());CheckFile(path,pcm,8);DeleteFileW(path.c_str());
 assert(BeginWaveExport(path,error));ArmForDevice(device);CaptureBlock(device,pcm,sizeof(pcm));
 originalReset=ResetOK;ExportReset(device);assert(!WaveExportActive());CheckFile(path,pcm,8);DeleteFileW(path.c_str());
 assert(BeginWaveExport(path,error));FinishWaveExport();assert(lastStatus.find(L"No audio")!=std::wstring::npos);
 // Preserve an existing destination on a no-audio attempt and on abort.
 FILE* existing=_wfopen(path.c_str(),L"wb");assert(existing);fputs("KEEP",existing);fclose(existing);
 assert(BeginWaveExport(path,error));auto recovery=temporary;ArmForDevice(device);CaptureBlock(device,pcm,sizeof(pcm));
 assert(Checkpoint());CheckFile(recovery,pcm,8);
 {std::lock_guard<std::mutex> guard(captureLock);FinishLocked(false);}
 existing=_wfopen(path.c_str(),L"rb");char marker[4];assert(fread(marker,1,4,existing)==4&&!memcmp(marker,"KEEP",4));fclose(existing);
 CheckFile(recovery,pcm,8);DeleteFileW(recovery.c_str());
 assert(BeginWaveExport(path,error));startedAt=GetTickCount64()-31000;assert(!WaveExportActive());
 assert(GetFileAttributesW(temporary.c_str())==INVALID_FILE_ATTRIBUTES);
 assert(!BeginWaveExport(path+L"\\missing\\export.wav",error));
 DeleteFileW(path.c_str());
 std::cout<<"Yamaha PCM export, WAV header/data, previous-track isolation and finalization passed.\n";
 return 0;
}
