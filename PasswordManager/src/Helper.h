#pragma once

#include <Windows.h>

#include <string>

std::wstring SanitizeCommandLine(const WCHAR * szCmdLine);

std::wstring GetRegSZValue(HKEY hKey, const WCHAR * szSubkey = nullptr, const WCHAR * name = nullptr);

std::wstring GetFilenameFromPath(const std::wstring& path);