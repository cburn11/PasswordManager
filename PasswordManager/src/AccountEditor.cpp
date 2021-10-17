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
BOOL g_fUnsavedChanges = FALSE;

namespace AccountEditor {

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);

	void UpdateEditWnds(HWND hwndEditor, const Account * paccount);

	LRESULT CALLBACK CustomCtrlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		WNDPROC priorproc = (WNDPROC) GetWindowLongPtr(hwnd, GWLP_USERDATA);

		switch( message ) {

		case WM_KEYDOWN:

			if( g_fDisableNavigation )	break;

			HWND hwndEditorWnd = GetParent(hwnd);

			if( wParam == VK_NEXT || wParam == VK_PRIOR ) {

				PostMessage(hwndEditorWnd, WM_KEYDOWN, wParam, lParam);

			} 

			break;

		}

		return CallWindowProc(priorproc, hwnd, message, wParam, lParam);
	}

	INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		switch( message ) {

			HANDLE_DLG_MSG(hwnd, WM_INITDIALOG, Cls_OnInitDialog);
			HANDLE_DLG_MSG(hwnd, WM_COMMAND, Cls_OnCommand);
			HANDLE_DLG_MSG(hwnd, WM_KEYDOWN, Cls_OnKey);

		}

		return FALSE;

	}

	std::wstring generate_password() {

		std::wstring password;
	
		auto fUsePasswordGenerator = g_pUserData->pSettings->getDWORD(L"fUsePasswordGenerator");
		if( fUsePasswordGenerator ) {
			HRESULT hr;
			BSTR bstrPassword;
			hr = g_pUserData->p_pswdgen_Application->GeneratePassword(&bstrPassword);
			if( S_OK == hr ) {
				password = bstrPassword;
				SysFreeString(bstrPassword);
			}
		} else {
			password = ::GenerateRandomString(16);
		}

		return password;
	}

	void UpdateEditWnds(HWND hwndEditor, const Account * paccount) {
		
		if( paccount ) {

			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_ID), paccount->getString(Account::Field::ID).c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_NAME), paccount->getString(Account::Field::NAME).c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_URL), paccount->getString(Account::Field::URL).c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_USERNAME), paccount->getString(Account::Field::USERNAME).c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_PASSWORD2), paccount->getString(Account::Field::PASSWORD).c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_DESCRIPTION), paccount->getString(Account::Field::DESCRIPTION).c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_USERNAMEFIELD), paccount->getString(Account::Field::USERNAMEFIELD).c_str());
			Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_PASSWORDFIELD), paccount->getString(Account::Field::PASSWORDFIELD).c_str());

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

				auto password = generate_password();
				Edit_SetText(GetDlgItem(hwndEditor, IDC_EDIT_PASSWORD2), password.c_str());
			}
		}

		g_fUnsavedChanges = FALSE;

		Button_Enable(GetDlgItem(hwndEditor, IDOK), FALSE);

	}

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

		g_fDisableNavigation = lParam ? FALSE : TRUE;
		
		const Account * paccount = (const Account *) lParam;
		UpdateEditWnds(hwnd, paccount);	


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

	inline std::wstring MakeEditValueIntoString(HWND hwnd) {
		auto cch = Edit_GetTextLength(hwnd);
		auto textbuffer = std::make_unique<WCHAR[]>(cch + 1);
		Edit_GetText(hwnd, textbuffer.get(), cch + 1);
		return std::wstring{ textbuffer.get() };
	}

	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

		Account * paccount = nullptr;		

		switch( id ) {

		case IDOK: {

			paccount = (Account*) new Account;
			Account& account = *paccount;

			account[L"id"] = MakeEditValueIntoString(GetDlgItem(hwnd, IDC_EDIT_ID));
			account[L"name"] = MakeEditValueIntoString(GetDlgItem(hwnd, IDC_EDIT_NAME));
			account[L"url"] = MakeEditValueIntoString(GetDlgItem(hwnd, IDC_EDIT_URL));
			account[L"username"] = MakeEditValueIntoString(GetDlgItem(hwnd, IDC_EDIT_USERNAME));
			account[L"password"] = MakeEditValueIntoString(GetDlgItem(hwnd, IDC_EDIT_PASSWORD2));
			account[L"description"] = MakeEditValueIntoString(GetDlgItem(hwnd, IDC_EDIT_DESCRIPTION));
			account[L"usernamefield"] = MakeEditValueIntoString(GetDlgItem(hwnd, IDC_EDIT_USERNAMEFIELD));
			account[L"passwordfield"] = MakeEditValueIntoString(GetDlgItem(hwnd, IDC_EDIT_PASSWORDFIELD));

			g_fUnsavedChanges = FALSE;
		}
				 //	Fall through to IDCANCEL

		case IDCANCEL:

			if( g_fUnsavedChanges ) {

				auto fSave = MessageBox(hwnd, L"Do you want to save changes before closing?",
					L"Unsaved changes", MB_ICONQUESTION | MB_YESNO);

				if( IDYES == fSave ) {

					// Simulate an OK (caption: Save) button press
					PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDOK, codeNotify), ( LPARAM ) hwndCtl);

					break;

				}

			}

			EndDialog(hwnd, ( INT_PTR ) paccount);

			break;

		case IDC_EDIT_ID:
		case IDC_EDIT_NAME:
		case IDC_EDIT_URL:
		case IDC_EDIT_USERNAME:
		case IDC_EDIT_PASSWORD2:
		case IDC_EDIT_DESCRIPTION:
		case IDC_EDIT_USERNAMEFIELD:
		case IDC_EDIT_PASSWORDFIELD:

			if( codeNotify == EN_CHANGE ) {
				Button_Enable(GetDlgItem(hwnd, IDOK), TRUE);
				g_fUnsavedChanges = TRUE;
			}

			break;

		case IDC_BUTTON_GENERATE_PASSWORD: {
			std::wstring password = generate_password();
			Edit_SetText(GetDlgItem(hwnd, IDC_EDIT_PASSWORD2), password.c_str());
			break;
		}

		} // switch

	}

	void Cls_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags) {

		HWND hwndParentApp = GetParent(hwnd);
		Account * pNextAccount = nullptr;

		UINT msg = 0;

		switch( vk ) {

		case VK_NEXT:
			msg = GET_NEXT_ACCOUNT;
			break;

		case VK_PRIOR:
			msg = GET_PRIOR_ACCOUNT;
			break;

		}

		if( msg > 0 ) {

			if( g_fUnsavedChanges && ( IDOK !=
				MessageBox(hwnd, L"Unsaved changes will be discarded.",
					L"Unsaved Changes", MB_OKCANCEL | MB_ICONEXCLAMATION) ) ) {

				return;
			}

			SendMessage(hwndParentApp, msg, 0, (LPARAM) &pNextAccount);

			if( pNextAccount ) {

				UpdateEditWnds(hwnd, pNextAccount);
			}
		}
	}	
}