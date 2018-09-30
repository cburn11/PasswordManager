#include <Windows.h>
#include <windowsx.h>

#include <string>

#include "CommonHeaders.h"

#include "resource.h"
#include "BrowserCommandDialog.h"

#define APPLICATION_SUBKEY	L"SOFTWARE\\Clifford\\PasswordManager"

void BrowserCommand::LoadFromRegistry() {

	HKEY hApplicationKey;
	auto lRes = RegOpenKey(HKEY_CURRENT_USER, APPLICATION_SUBKEY, &hApplicationKey);
	if( ERROR_SUCCESS == lRes ) {

		WCHAR * szBrowserPath, *szParameters;
		
		DWORD type, cbStr;

		lRes = lRes = RegGetValue(hApplicationKey, nullptr, L"BrowserPath", RRF_RT_REG_SZ,
			&type, nullptr, &cbStr);
		if( ERROR_SUCCESS == lRes ) {
			szBrowserPath = ( WCHAR * ) new WCHAR[cbStr]{ 0 };
			lRes = lRes = RegGetValue(hApplicationKey, nullptr, L"BrowserPath", RRF_RT_REG_SZ,
				&type, szBrowserPath, &cbStr);
			m_browserpath = szBrowserPath;
			delete[] szBrowserPath;
		}

		lRes = lRes = RegGetValue(hApplicationKey, nullptr, L"Parameters", RRF_RT_REG_SZ,
			&type, nullptr, &cbStr);
		if( ERROR_SUCCESS == lRes ) {
			szParameters = ( WCHAR * ) new WCHAR[cbStr]{ 0 };
			lRes = lRes = RegGetValue(hApplicationKey, nullptr, L"Parameters", RRF_RT_REG_SZ,
				&type, szParameters, &cbStr);
			m_parameters = szParameters;
			delete[] szParameters;
		}

		RegCloseKey(hApplicationKey);

	}
}

void BrowserCommand::SaveToRegistry() {

	HKEY hApplicationKey;
	auto lRes = RegOpenKey(HKEY_CURRENT_USER, APPLICATION_SUBKEY, &hApplicationKey);
	if( ERROR_SUCCESS == lRes ) {

		lRes = RegSetValueEx(hApplicationKey, L"BrowserPath", 0, REG_SZ,
			(const BYTE *) (m_browserpath.c_str()), ( m_browserpath.size() + 1 ) * sizeof(WCHAR));

		lRes = RegSetValueEx(hApplicationKey, L"Parameters", 0, REG_SZ,
			(const BYTE *) ( m_parameters.c_str() ), ( m_parameters.size() + 1 ) * sizeof(WCHAR));

		RegCloseKey(hApplicationKey);
	}
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