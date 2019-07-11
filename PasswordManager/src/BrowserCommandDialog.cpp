#include <Windows.h>
#include <windowsx.h>

#include <string>

#include "CommonHeaders.h"
#include "Helper.h"

#include "resource.h"
#include "BrowserCommandDialog.h"

#define APPLICATION_SUBKEY	L"SOFTWARE\\Clifford\\PasswordManager"

void BrowserCommand::LoadFromRegistry() {
		
	auto szBrowserPath = m_pSettings->getSZ(L"BrowserPath");
	if( szBrowserPath != nullptr )	m_browserpath = szBrowserPath;
	
	auto szParameters = m_pSettings->getSZ(L"Parameters");
	if( szParameters != nullptr )	m_parameters = szParameters;
}

void BrowserCommand::SaveToRegistry() {

	CComBSTR browserpath{ m_browserpath.c_str() };
	m_pSettings->setSZ(L"BrowserPath", browserpath);

	CComBSTR parameters{ m_parameters.c_str() };
	m_pSettings->setSZ(L"Parameters", parameters);

	std::wstring filename = GetFilenameFromPath(m_defaultbrowserpath);
	std::wstring param_name = filename + L"_params";
	CComBSTR browser_default_param{ m_defaultparameters.c_str() };
	m_pSettings->setSZ(param_name.c_str(), browser_default_param);
}

std::wstring BrowserCommand::GetDefaultBrowserFromRegistry() {

	auto szkeyUserChoice = L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice";
	
	// Get default browser ProgId
	std::wstring ProgId = GetRegSZValue(HKEY_CURRENT_USER, szkeyUserChoice, L"ProgID");

	// Get Open command form default browser ProgId
	const std::wstring browser_open_command_key = ProgId + L"\\shell\\open\\command";
	std::wstring unsanitized_default_browser = GetRegSZValue(HKEY_CLASSES_ROOT, browser_open_command_key.c_str());
	std::wstring default_browser = SanitizeCommandLine(unsanitized_default_browser.c_str());

	return default_browser;
}

void BrowserCommand::LoadDefaultBrowserParametersFromRegistry(const std::wstring& default_browser_path) {

	std::wstring filename = GetFilenameFromPath(default_browser_path);

	std::wstring param_name = filename + L"_params";
	auto param = m_pSettings->getSZ(param_name.c_str());
	if( param ) m_defaultparameters = param;
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
		HWND hwndDefaultParams = GetDlgItem(hwnd, IDC_EDIT_DEFAULT_PARAMETERS);

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

			memset(textbuffer, 0, 1024);
			
			Edit_GetText(hwndDefaultParams, textbuffer, 1024);
			std::wstring default_params{ textbuffer };
			pBrowserCommand->SetDefaultParameters(default_params);

			// Fall through

		} case IDCANCEL:

			EndDialog(hwnd, 0);

			break;

		case IDC_EDIT_BROWSER_PATH:
		case IDC_EDIT_PARAMETERS:
		case IDC_EDIT_DEFAULT_PARAMETERS:

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
		HWND hwndDefaultPath = GetDlgItem(hwnd, IDC_EDIT_DEFAULT_BROWSERPATH);
		HWND hwndDefaultParams = GetDlgItem(hwnd, IDC_EDIT_DEFAULT_PARAMETERS);

		Edit_SetText(hwndPath, pBrowserCommand->GetBrowserPath().c_str());
		Edit_SetText(hwndParams, pBrowserCommand->GetParameters().c_str());
		Edit_SetText(hwndDefaultPath, pBrowserCommand->GetDefaultBrowserPath().c_str());
		Edit_SetText(hwndDefaultParams, pBrowserCommand->GetDefaultParameters().c_str());

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