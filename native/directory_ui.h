#pragma once
#include <windows.h>
#include <string>
#include <utility>
#include <vector>

// Implemented by the playback adapter. Return false if the request was rejected.
bool OpenTrackURL(const std::wstring& url, const std::wstring& title);
bool OpenTrackPlaylist(const std::vector<std::pair<std::wstring, std::wstring>>& tracks,
                       const std::wstring& title);
void StartDirectoryUI(HMODULE module);
void NavigateDirectory(const std::wstring& url);
void SetDirectoryStatus(const std::wstring& text);
bool ExportTrackWav(const std::wstring& url, const std::wstring& title, const std::wstring& destination);
