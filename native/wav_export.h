#pragma once
#include <string>
bool InitializeWaveExport(const std::wstring& applicationFolder);
bool BeginWaveExport(const std::wstring& destination, std::wstring& error);
bool WaveExportActive();
void FinishWaveExport();
