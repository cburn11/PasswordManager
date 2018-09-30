#include <Windows.h>
#include <windowsx.h>

#include "CommonHeaders.h"
#include "AboutDialog.h"

namespace AboutDialog {

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

		return TRUE;
	}

	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

		switch( id ) {

		case IDCANCEL:
		case IDOK:

			EndDialog(hwnd, 0);

			break;
		}

	}

	INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		switch( message ) {

			HANDLE_DLG_MSG(hwnd, WM_INITDIALOG, Cls_OnInitDialog);
			HANDLE_DLG_MSG(hwnd, WM_COMMAND, Cls_OnCommand);

		}

		return FALSE;

	}

}