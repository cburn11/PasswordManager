#include <Windows.h>
#include <windowsx.h>

#include <string>
#include <memory>

#include "CommonHeaders.h"

#include "resource.h"

#include "AccountEditor.h"

#include "PasswordManager.h"
#include "EncryptionHelper.h"

using std::wstring;

BOOL g_fDisableNavigation = FALSE;

namespace AccountEditor {

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	void UpdateEditWnds(HWND hwndEditor, const Account * paccount);

	LRESULT CALLBACK CustomCtrlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		WNDPROC priorproc = (WNDPROC) GetWindowLongPtr(hwnd, GWLP_USERDATA);

		switch( message ) {

		case WM_KEYDOWN:

			if( g_fDisableNavigation )	break;

			HWND hwndEditorWnd = GetParent(hwnd);
			HWND hwndParentApp = GetParent(hwndEditorWnd);
			Account * pNextAccount = nullptr;
			if( wParam == VK_NEXT ) {

				//EndDialog(hwndEditorWnd, LAUNCH_EDITOR_NEXT);
				SendMessage(hwndParentApp, GET_NEXT_ACCOUNT, 0, (LPARAM) &pNextAccount);

			} else if( wParam == VK_PRIOR ) {

				//EndDialog(hwndEditorWnd, LAUNCH_EDITOR_PRIOR);
				SendMessage(hwndParentApp, GET_PRIOR_ACCOUNT, 0, (LPARAM) &pNextAccount);

			}

			if( pNextAccount ) {

				UpdateEditWnds(hwndEditorWnd, pNextAccount);

			}

			break;

		}

		return CallWindowProc(priorproc, hwnd, message, wParam, lParam);
	}

	INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		switch( message ) {

			HANDLE_DLG_MSG(hwnd, WM_INITDIALOG, Cls_OnInitDialog);
			HANDLE_DLG_MSG(hwnd, WM_COMMAND, Cls_OnCommand);

		}

		return FALSE;

	}

	void UpdateEditWnds(HWND hwndEditor, const Account * paccount) {
		
		if( paccount ) {

			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_ID), paccount->id.c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_NAME), paccount->name.c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_URL), paccount->url.c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_USERNAME), paccount->username.c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_PASSWORD2), paccount->password.c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_DESCRIPTION), paccount->description.c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_USERNAMEFIELD), paccount->usernamefield.c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_PASSWORDFIELD), paccount->passwordfield.c_str());

		} else {
			//	Added 1.1.18 to auto populate the ID field for NEW accounts

			HWND hwndParent = GetParent(hwndEditor);
			if( !hwndParent )
				return;

			UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
			if( pUserData ) {
				const std::vector<Account>& accounts = pUserData->pAccounts->GetAccounts();

				std::wstring strRand{ ::GenerateRandomString(8) };
				Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_ID), strRand.c_str());
			}
		}

	}

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
		
		g_fDisableNavigation = lParam ? FALSE : TRUE;

		const Account * paccount = (const Account *) lParam;
		UpdateEditWnds(hwnd, paccount);

		Button_Enable(GetDlgItem(hwnd, IDOK), FALSE);

		int focus_target = paccount ? IDC_EDIT_ID : IDC_EDIT_NAME;
		SetFocus(GetDlgItem(hwnd, focus_target));

		for( int index = IDC_EDIT_ID; index <= IDC_EDIT_PASSWORDFIELD; ++index ) {
			HWND hwndCtrl = GetDlgItem(hwnd, index);
			auto priorproc = SetWindowLongPtr(hwndCtrl, GWLP_WNDPROC, (LONG_PTR) CustomCtrlProc);
			SetWindowLongPtr(hwndCtrl, GWLP_USERDATA, priorproc);
		}

		return FALSE;
	}

	inline void CopyEditValueIntoAccount(HWND hwnd, wstring& str) {
		auto cch = Edit_GetTextLength(hwnd);
		auto textbuffer = std::make_unique<WCHAR[]>(cch + 1);
		Edit_GetText(hwnd, textbuffer.get(), cch+1);
		str = textbuffer.get();
	}

	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

		Account * paccount = nullptr;		

		switch( id ) {

		case IDOK:

			paccount = ( Account * ) new Account;
			
			CopyEditValueIntoAccount(GetDlgItem(hwnd, IDC_EDIT_ID), paccount->id);
			CopyEditValueIntoAccount(GetDlgItem(hwnd, IDC_EDIT_NAME), paccount->name);
			CopyEditValueIntoAccount(GetDlgItem(hwnd, IDC_EDIT_URL), paccount->url);
			CopyEditValueIntoAccount(GetDlgItem(hwnd, IDC_EDIT_USERNAME), paccount->username);
			CopyEditValueIntoAccount(GetDlgItem(hwnd, IDC_EDIT_PASSWORD2), paccount->password);
			CopyEditValueIntoAccount(GetDlgItem(hwnd, IDC_EDIT_DESCRIPTION), paccount->description);
			CopyEditValueIntoAccount(GetDlgItem(hwnd, IDC_EDIT_USERNAMEFIELD), paccount->usernamefield);
			CopyEditValueIntoAccount(GetDlgItem(hwnd, IDC_EDIT_PASSWORDFIELD), paccount->passwordfield);
			
			//	Fall through to IDCANCEL

		case IDCANCEL:
			EndDialog(hwnd, (INT_PTR) paccount);
			break;

		case IDC_EDIT_ID:
		case IDC_EDIT_NAME:
		case IDC_EDIT_URL:
		case IDC_EDIT_USERNAME:
		case IDC_EDIT_PASSWORD2:
		case IDC_EDIT_DESCRIPTION:
		case IDC_EDIT_USERNAMEFIELD:
		case IDC_EDIT_PASSWORDFIELD:

			if( codeNotify == EN_CHANGE )
				Button_Enable(GetDlgItem(hwnd, IDOK), TRUE);

			break;

		}

	}
	
}