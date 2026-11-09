// Capture only this player's submitted PCM, never system/microphone loopback.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <map>
#include <mutex>
#include <string>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <io.h>
#include "wav_export.h"
#include "directory_ui.h"
#include "runtime_log.h"

namespace {
using OpenFn=MMRESULT(WINAPI*)(LPHWAVEOUT,UINT,LPCWAVEFORMATEX,DWORD_PTR,DWORD_PTR,DWORD);
using WriteFn=MMRESULT(WINAPI*)(HWAVEOUT,LPWAVEHDR,UINT);
using DeviceFn=MMRESULT(WINAPI*)(HWAVEOUT);
OpenFn originalOpen=nullptr;
WriteFn originalWrite=nullptr;
DeviceFn originalReset=nullptr,originalClose=nullptr,originalRestart=nullptr;
std::mutex captureLock;
std::map<HWAVEOUT,WAVEFORMATEX> formats;
std::once_flag initializeOnce;
bool hooksReady=false,active=false,ready=false;
HWAVEOUT capturedDevice=nullptr;
FILE* recording=nullptr;
WAVEFORMATEX recordedFormat={};
uint32_t recordedBytes=0;
std::wstring destination,temporary;
ULONGLONG startedAt=0;
uint32_t checkpointBytes=0;
constexpr uint64_t MaxExportBytes=1024ULL*1024*1024;

bool SupportedPCM(const WAVEFORMATEX& format){
 return format.wFormatTag==WAVE_FORMAT_PCM&&format.nChannels>=1&&format.nChannels<=8
     &&format.nSamplesPerSec>=8000&&format.nSamplesPerSec<=192000
     &&format.wBitsPerSample>=8&&format.wBitsPerSample<=32&&format.wBitsPerSample%8==0
     &&format.nBlockAlign==format.nChannels*(format.wBitsPerSample/8)
     &&format.nAvgBytesPerSec==format.nSamplesPerSec*format.nBlockAlign;
}
bool Checkpoint(){
 if(!recording)return false;
 unsigned char header[44]={};
 auto u16=[&](int at,uint16_t v){header[at]=v;header[at+1]=v>>8;};
 auto u32=[&](int at,uint32_t v){for(int i=0;i<4;++i)header[at+i]=v>>(i*8);};
 memcpy(header,"RIFF",4);u32(4,36+recordedBytes);memcpy(header+8,"WAVEfmt ",8);u32(16,16);
 u16(20,WAVE_FORMAT_PCM);u16(22,recordedFormat.nChannels);u32(24,recordedFormat.nSamplesPerSec);
 u32(28,recordedFormat.nAvgBytesPerSec);u16(32,recordedFormat.nBlockAlign);u16(34,recordedFormat.wBitsPerSample);
 memcpy(header+36,"data",4);u32(40,recordedBytes);
 bool ok=fseek(recording,0,SEEK_SET)==0&&fwrite(header,1,44,recording)==44&&fflush(recording)==0;
 if(fseek(recording,44+recordedBytes,SEEK_SET))ok=false;
 checkpointBytes=recordedBytes;return ok;
}
std::wstring FinishLocked(bool publish=true){
 active=false;ready=false;capturedDevice=nullptr;
 if(!recording){DeleteFileW(temporary.c_str());return L"No audio captured. Use Yamaha's built-in software synthesizer, then Export WAV again.";}
 bool ok=Checkpoint();
 // Discard an incomplete sample at the tail after a short disk write.
 if(_chsize(_fileno(recording),44+recordedBytes))ok=false;
 if(fclose(recording))ok=false;recording=nullptr;
 if(ok&&publish&&recordedBytes&&MoveFileExW(temporary.c_str(),destination.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
  return L"Saved WAV: "+destination;
 runtimeLog("WAV export incomplete; partial retained, final destination unchanged");
 return L"WAV export incomplete. Original destination preserved; recovery file: "+temporary;
}
void ArmForDevice(HWAVEOUT device){
 std::lock_guard<std::mutex> guard(captureLock);
 if(active&&!recordedBytes){ready=true;capturedDevice=nullptr;}
}
void CaptureBlock(HWAVEOUT device,const void* bytes,DWORD length){
 if(!bytes||!length)return;
 std::lock_guard<std::mutex> guard(captureLock);
 if(!active||!ready)return;
 auto format=formats.find(device);
 if(format==formats.end()||(capturedDevice&&device!=capturedDevice))return;
 if(!SupportedPCM(format->second)){
  if(recording)FinishLocked(false);
  active=false;ready=false;SetDirectoryStatus(L"WAV export requires PCM audio from Yamaha's built-in software synth.");return;
 }
 if(!recording){
  recording=_wfopen(temporary.c_str(),L"rb+");
  if(!recording){active=false;ready=false;SetDirectoryStatus(L"Could not create the selected WAV file.");return;}
  unsigned char header[44]={};
  if(fwrite(header,1,44,recording)!=44){fclose(recording);recording=nullptr;active=false;ready=false;SetDirectoryStatus(L"Could not write the WAV header.");return;}
  recordedFormat=format->second;capturedDevice=device;
  runtimeLog("WAV PCM capture started: rate=%lu channels=%u bits=%u",recordedFormat.nSamplesPerSec,recordedFormat.nChannels,recordedFormat.wBitsPerSample);
  SetDirectoryStatus(L"Rendering to WAV in real time. Click Finish export when the track ends.");
 }
 if(recordedFormat.nSamplesPerSec!=format->second.nSamplesPerSec||recordedFormat.nChannels!=format->second.nChannels||recordedFormat.wBitsPerSample!=format->second.wBitsPerSample){
  SetDirectoryStatus(FinishLocked());return;
 }
 if(length%recordedFormat.nBlockAlign||uint64_t(recordedBytes)+length>MaxExportBytes){
  FinishLocked(false);SetDirectoryStatus(L"Export reached the size/format limit. Recovery WAV retained; destination unchanged.");return;
 }
 size_t written=fwrite(bytes,1,length,recording);
 recordedBytes+=uint32_t(written-written%recordedFormat.nBlockAlign);
 if(written!=length){FinishLocked(false);SetDirectoryStatus(L"Disk write failed. Destination preserved; check the partial WAV.");}
 else if(recordedBytes-checkpointBytes>=recordedFormat.nAvgBytesPerSec*2){
  if(!Checkpoint()){FinishLocked(false);SetDirectoryStatus(L"WAV checkpoint failed. Destination preserved.");}
 }
}
MMRESULT WINAPI ExportOpen(LPHWAVEOUT out,UINT device,LPCWAVEFORMATEX format,DWORD_PTR callback,DWORD_PTR instance,DWORD flags){
 MMRESULT result=originalOpen(out,device,format,callback,instance,flags);
 if(result==MMSYSERR_NOERROR&&!(flags&WAVE_FORMAT_QUERY)&&out&&format){
  runtimeLog("WAV output opened: tag=%u rate=%lu channels=%u bits=%u",format->wFormatTag,format->nSamplesPerSec,format->nChannels,format->wBitsPerSample);
  {std::lock_guard<std::mutex> guard(captureLock);formats[*out]=*format;}
  ArmForDevice(*out);
 }
 return result;
}
MMRESULT WINAPI ExportWrite(HWAVEOUT device,LPWAVEHDR header,UINT size){
 std::vector<unsigned char> copy;
 bool exporting=false;{std::lock_guard<std::mutex> guard(captureLock);exporting=active&&ready;}
 // Yamaha can recycle its buffer after the device callback; snapshot it before
 // submission, then record it only if the real write succeeds.
 try {
 if(exporting&&header&&header->lpData&&header->dwBufferLength&&header->dwBufferLength<=1024UL*1024)
  copy.assign(reinterpret_cast<unsigned char*>(header->lpData),reinterpret_cast<unsigned char*>(header->lpData)+header->dwBufferLength);
 }catch(...){
  std::lock_guard<std::mutex> guard(captureLock);
  FinishLocked(false);runtimeLog("WAV capture allocation failed; playback continues");
 }
 MMRESULT result=originalWrite(device,header,size);
 try {
  if(result==MMSYSERR_NOERROR&&!copy.empty())CaptureBlock(device,copy.data(),DWORD(copy.size()));
  else if(exporting&&header&&header->dwBufferLength>1024UL*1024){
   std::lock_guard<std::mutex> guard(captureLock);FinishLocked(false);
   SetDirectoryStatus(L"Export stopped: unusually large audio buffer. Destination preserved.");
  }
 }catch(...){runtimeLog("WAV capture exception; playback continues");}
 return result;
}
MMRESULT WINAPI ExportReset(HWAVEOUT device){
 MMRESULT result=originalReset(device);
 std::lock_guard<std::mutex> guard(captureLock);
 if(active){if(recordedBytes&&capturedDevice==device)SetDirectoryStatus(FinishLocked());else if(!recordedBytes)ready=true;}
 return result;
}
MMRESULT WINAPI ExportClose(HWAVEOUT device){
 MMRESULT result=originalClose(device);
 std::lock_guard<std::mutex> guard(captureLock);
 if(result==MMSYSERR_NOERROR){
  if(active&&recordedBytes&&capturedDevice==device)SetDirectoryStatus(FinishLocked());
  formats.erase(device);
 }
 return result;
}
MMRESULT WINAPI ExportRestart(HWAVEOUT device){
 MMRESULT result=originalRestart(device);if(result==MMSYSERR_NOERROR)ArmForDevice(device);return result;
}
int HookModule(HMODULE module){
 auto base=reinterpret_cast<unsigned char*>(module);auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
 if(!base||dos->e_magic!=IMAGE_DOS_SIGNATURE)return 0;
 auto nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);
 if(nt->Signature!=IMAGE_NT_SIGNATURE)return 0;
 DWORD rva=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
 if(!rva)return 0;int count=0;
 auto libraries=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+rva);
 for(;libraries->Name;++libraries){
  if(_stricmp(reinterpret_cast<char*>(base+libraries->Name),"winmm.dll")||!libraries->OriginalFirstThunk)continue;
  auto names=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+libraries->OriginalFirstThunk);
  auto slots=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+libraries->FirstThunk);
  for(;names->u1.AddressOfData;++names,++slots){
   if(IMAGE_SNAP_BY_ORDINAL32(names->u1.Ordinal))continue;
   auto name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData);
   void* replacement=nullptr;
   if(!strcmp(reinterpret_cast<char*>(name->Name),"waveOutOpen"))replacement=reinterpret_cast<void*>(ExportOpen);
   else if(!strcmp(reinterpret_cast<char*>(name->Name),"waveOutWrite"))replacement=reinterpret_cast<void*>(ExportWrite);
   else if(!strcmp(reinterpret_cast<char*>(name->Name),"waveOutReset"))replacement=reinterpret_cast<void*>(ExportReset);
   else if(!strcmp(reinterpret_cast<char*>(name->Name),"waveOutClose"))replacement=reinterpret_cast<void*>(ExportClose);
   else if(!strcmp(reinterpret_cast<char*>(name->Name),"waveOutRestart"))replacement=reinterpret_cast<void*>(ExportRestart);
   if(!replacement)continue;DWORD old=0;
   if(!VirtualProtect(&slots->u1.Function,sizeof(slots->u1.Function),PAGE_READWRITE,&old))continue;
   InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&slots->u1.Function),replacement);
   DWORD ignored;VirtualProtect(&slots->u1.Function,sizeof(slots->u1.Function),old,&ignored);++count;
  }
 }
 return count;
}
}

bool InitializeWaveExport(const std::wstring& folder){
 std::call_once(initializeOnce,[&](){
  HMODULE winmm=LoadLibraryW(L"winmm.dll");if(!winmm)return;
  originalOpen=reinterpret_cast<OpenFn>(GetProcAddress(winmm,"waveOutOpen"));
  originalWrite=reinterpret_cast<WriteFn>(GetProcAddress(winmm,"waveOutWrite"));
  originalReset=reinterpret_cast<DeviceFn>(GetProcAddress(winmm,"waveOutReset"));
  originalClose=reinterpret_cast<DeviceFn>(GetProcAddress(winmm,"waveOutClose"));
  originalRestart=reinterpret_cast<DeviceFn>(GetProcAddress(winmm,"waveOutRestart"));
  if(!originalOpen||!originalWrite||!originalReset||!originalClose||!originalRestart)return;
  int patched=0;
  for(auto name:{L"WaveOut.dll",L"MidiOut.dll",L"Smaf.dll"}){
   // Keep the patched output module loaded for the player's lifetime.
   HMODULE module=LoadLibraryW((folder+L"\\Addins\\"+name).c_str());
   if(module)patched+=HookModule(module);
  }
  hooksReady=patched>=5;
  runtimeLog("WAV export output hooks installed: %d",patched);
 });
 return hooksReady;
}
bool BeginWaveExport(const std::wstring& output,std::wstring& error){
 std::lock_guard<std::mutex> guard(captureLock);
 if(!hooksReady){error=L"Yamaha's WAV output is unavailable; export cannot be started.";return false;}
 if(active){error=L"Finish the current WAV export first.";return false;}
 if(output.empty()){error=L"Choose a WAV destination.";return false;}
 // Reserve a unique sibling file now: failure must precede starting playback.
 temporary=output+L".partial-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64())+L".wav";
 HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE){error=L"Cannot create a WAV beside this destination. Check folder permissions and free space.";return false;}
 CloseHandle(file);
 destination=output;active=true;ready=false;recordedBytes=0;checkpointBytes=0;capturedDevice=nullptr;recording=nullptr;startedAt=GetTickCount64();
 return true;
}
bool WaveExportActive(){
 std::lock_guard<std::mutex> guard(captureLock);
 if(active&&!recordedBytes&&GetTickCount64()-startedAt>30000){
  SetDirectoryStatus(FinishLocked(false));DeleteFileW(temporary.c_str());
 }
 return active;
}
void FinishWaveExport(){std::lock_guard<std::mutex> guard(captureLock);if(active)SetDirectoryStatus(FinishLocked());}
