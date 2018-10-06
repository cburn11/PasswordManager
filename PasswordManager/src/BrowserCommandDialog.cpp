#include <Windows.h>
#include <windowsx.h>

#include <string>

#include "CommonHeaders.h"

#include "resource.h"
#include "BrowserCommandDialog.h"

#define APPLICATION_SUBKEY	L"SOFTWARE\\Clifford\\PasswordManager"

void BrowserCommand::LoadFromRegistry() {
		
	m_browserpath = m_pSettings->getSZ(L"BrowserPath");
	
	m_parameters = m_pSettings->getSZ(L"Parameters");
}

void BrowserCommand::SaveToRegistry() {

	CComBSTR browserpath{ m_browserpath.c_str() };
	m_pSettings->setSZ(L"BrowserPath", browserpath);

	CComBSTR parameters{ m_parameters.c_str() };
	m_pSettings->setSZ(L"Parameters", parameters);
}

namespace BrowserCommandDialog {

	bool GetBrowserPath(HWND hwnd, WCHAR ** pszPath) {
		
		WCHAR			* szFilename = nullptr;
		OPENFILENAME	ofn = { sizeof(ofn), 0 };

		szFilename = new WCHAR[MAX_PATH]{ 0 };
		if( !szFilename ) {
			*pszPath = nullptr;
			return false;
		}

		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFilename;
		ofn.nMaxFile = sizeof(WCHAR) * MAX_PATH;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		DWORD error;
		BOOL ret;

		ret = GetOpenFileName(&ofn);
		if( !ret ) {
			*pszPath = nullptr;
			delete szFilename;
			return false;
		}

		*pszPath = szFilename;

		return true;
	}

	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

		auto pBrowserCommand = (BrowserCommand *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

		HWND hwndPath = GetDlgItem(hwnd, IDC_EDIT_BROWSER_PATH);
		HWND hwndParams = GetDlgItem(hwnd, IDC_EDIT_PARAMETERS);

		switch( id ) {

		case IDC_BUTTON_BROWSE: {

			WCHAR * szPath = nullptr;
			if( GetBrowserPath(hwnd, &szPath) ) {
				Edit_SetText(hwndPath, szPath);
			}

			if( szPath )	delete szPath;

			break;

		} case IDOK: {

			WCHAR textbuffer[1024]{ 0 };

			Edit_GetText(hwndPath, textbuffer, 1024);
			std::wstring browserpath{ textbuffer };
			pBrowserCommand->SetBrowserPath(browserpath);

			memset(textbuffer, 0, 1024);

			Edit_GetText(hwndParams, textbuffer, 1024);
			std::wstring params{ textbuffer };
			pBrowserCommand->SetParameters(params);

			// Fall through

		} case IDCANCEL:

			EndDialog(hwnd, 0);

			break;

		case IDC_EDIT_BROWSER_PATH:
		case IDC_EDIT_PARAMETERS:

			if( codeNotify == EN_CHANGE )
				Button_Enable(GetDlgItem(hwnd, IDOK), TRUE);

			break;

		}

	}

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

		BrowserCommand * pBrowserCommand = (BrowserCommand *) lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) lParam);

		HWND hwndPath = GetDlgItem(hwnd, IDC_EDIT_BROWSER_PATH);
		HWND hwndParams = GetDlgItem(hwnd, IDC_EDIT_PARAMETERS);

		Edit_SetText(hwndPath, pBrowserCommand->GetBrowserPath().c_str());
		Edit_SetText(hwndParams, pBrowserCommand->GetParameters().c_str());

		Button_Enable(GetDlgItem(hwnd, IDOK), FALSE);

		return TRUE;
	}

	INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		switch( message ) {

			HANDLE_DLG_MSG(hwnd, WM_COMMAND, Cls_OnCommand);
			HANDLE_DLG_MSG(hwnd, WM_INITDIALOG, Cls_OnInitDialog);

		}

		return FALSE;

	}

}