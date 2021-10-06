#pragma once

#include <vector>
#include <string>
#include <utility>

#define WM_CM_INCREMENT (WM_USER + 3)

namespace ClipboardMonitor {

	INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	using CM_string_pair = std::pair<std::wstring, bool>;

}