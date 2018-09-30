#pragma once

#define WM_CM_INCREMENT (WM_USER + 3)

namespace ClipboardMonitor {

	INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

}