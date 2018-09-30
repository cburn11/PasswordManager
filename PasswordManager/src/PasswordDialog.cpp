#include <Windows.h>
#include <windowsx.h>

#include "CommonHeaders.h"
#include "resource.h"
#include "PasswordDialog.h"

namespace PasswordDialog {

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

		SetFocus(GetDlgItem(hwnd, IDC_EDIT_PASSWORD));

		return FALSE;
	}

	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

		WCHAR * szPassword = nullptr;

		HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_PASSWORD);

		switch( id ) {

		case IDOK: {

			int cch = Edit_GetTextLength(hwndEdit);
			szPassword = new WCHAR[cch + 1]{ 0 };
			if( szPassword )	Edit_GetText(hwndEdit, szPassword, cch + 1);
		}

		case IDCANCEL:		

			EndDialog(hwnd, (INT_PTR) szPassword);

			break;
		}

	}

	INT_PTR CALLBACK PasswordDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		switch( message ) {

			HANDLE_DLG_MSG(hwnd, WM_INITDIALOG, Cls_OnInitDialog);
			HANDLE_DLG_MSG(hwnd, WM_COMMAND, Cls_OnCommand);

		}

		return FALSE;

	}

}