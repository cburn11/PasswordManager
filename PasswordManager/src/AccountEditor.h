#pragma once

#define LAUNCH_EDITOR_NEXT	0x0001
#define LAUNCH_EDITOR_PRIOR 0x0002

#define GET_NEXT_ACCOUNT (WM_USER + 1)
#define GET_PRIOR_ACCOUNT (WM_USER + 2)

namespace AccountEditor {
	
	INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

}