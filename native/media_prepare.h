#pragma once
#include <string>
bool PrepareMedia(const std::wstring& source, const std::wstring& applicationFolder,
                  std::wstring& localFile, std::wstring& error);
