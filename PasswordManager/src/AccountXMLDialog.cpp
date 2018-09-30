#include <Windows.h>
#include <windowsx.h>

#include "AccountXMLDialog.h"
#include "resource.h"
#include "CommonHeaders.h"
#include "WindowClass.h"

AccountXMLDialog::AccountXMLDialog(const std::wstring& xml) :
	m_xml{ xml } {

	m_hInstance = GetModuleHandle(NULL);
	m_szTemplate = MAKEINTRESOURCE(IDD_DIALOG_ACCOUNTXML);
}

INT_PTR AccountXMLDialog::MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam) {

	auto ret = __super::MessageHandler(message, wParam, lParam);

	switch( message ) {

		HANDLE_DLG_MSG(m_window, WM_COMMAND, Cls_OnCommand);
		HANDLE_DLG_MSG(m_window, WM_INITDIALOG, Cls_OnInitDialog);

	}

	return ret;
}

BOOL AccountXMLDialog::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

	HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_ACCOUNTXML);
	Edit_SetText(hwndEdit, m_xml.c_str());
	Edit_FmtLines(hwndEdit, TRUE);

	auto len = Edit_GetTextLength(hwndEdit);
	Edit_SetSel(hwndEdit, 0, len);

	SetFocus(hwndEdit);

	return FALSE;
}

void AccountXMLDialog::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

	switch( id ) {

	case IDOK:
		EndDialog(hwnd, 0);
		break;

	}
}