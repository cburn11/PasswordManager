#include <Windows.h>
#include <windowsx.h>
#include <commdlg.h>
//#include <MsXml6.h>
//#include <ExDisp.h>
//#include <MsHTML.h>
#include <ShlObj.h>

#include <atlcomcli.h>

#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <memory>

#include "Helper.h"

#include "Accounts.h"
#include "AccountEditor.h"
#include "ClipboardMonitor.h"
#include "RecentFiles.h"
#include "BrowserCommandDialog.h"
#include "ClipboardMonitor.h"
#include "resource.h"
#include "App.h"
#include "Tools.h"
#include "PasswordManager.h"
#include "AboutDialog.h"
#include "AccountXMLDialog.h"
#include "SearchDialog.h"
#include "EncryptionHelper.h"

#define UPDATE_SELECTED_ACCOUNT 0xE001

using std::wstring;
using std::to_wstring;

using std::for_each;
using std::end;
using std::begin;

using std::vector;

bool GetSelectedAccount(HWND hwndListBox, const Account ** ppAcount);

void UpdateDisplay(HWND hwnd) {

	const Account * pAccount = nullptr;
	GetSelectedAccount(hwnd, &pAccount);

	ListBox_ResetContent(hwnd);

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !( pUserData->pAccounts ) )
		return;

	const vector<Account>& accounts = pUserData->pAccounts->GetAccounts();

	auto DisplayAccount = [hwnd](const Account& account) {

		ListBox_AddString(hwnd, account.to_string().c_str());
	};

	for_each(begin(accounts), end(accounts), DisplayAccount);

	if( pAccount ) {

		auto sel = pAccount->index;
		ListBox_SetCurSel(hwnd, sel);
	}
}

void ExitApplication(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( pUserData->pSettings ) {
		
		RECT rect;
		if( GetWindowRect(hwnd, &rect) ) {

			if( rect.left > 0 && rect.top > 0 ) {

				pUserData->pSettings->setDWORD(L"height", rect.bottom - rect.top);
				pUserData->pSettings->setDWORD(L"width", rect.right - rect.left);
				pUserData->pSettings->setDWORD(L"x", rect.left);
				pUserData->pSettings->setDWORD(L"y", rect.top);
			}
		}
	}

	auto hr = RevokeDragDrop(hwnd);

	PostQuitMessage(0);
}

enum GetFilenameType {
	open	= 1,
	save	= 2,
	saveEnc	= 3
};

bool GetFilename(HWND hwndParent, GetFilenameType type, const WCHAR ** pszFilename) {
	WCHAR			* szFilename = nullptr;
	OPENFILENAME	ofn = { sizeof(ofn), 0 };

	szFilename = new WCHAR[MAX_PATH] {0};
	if( !szFilename ) {
		*pszFilename = nullptr;
		return false;
	}

	ofn.hwndOwner = hwndParent;
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = sizeof(WCHAR) * MAX_PATH;
	
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	DWORD error;
	BOOL ret;
	if( type == GetFilenameType::open ) {
		ofn.lpstrFilter = L"Password Manager data file\0*.pwm\0xml\0*.xml";
		ofn.nFileExtension = 1;
		ret = GetOpenFileName(&ofn);
	} else if( type==GetFilenameType::save ) {
		ofn.lpstrFilter = L"xml\0*.xml\0";
		ofn.nFileExtension = 1;
		ret = GetSaveFileName(&ofn);
	} else {
		ofn.lpstrFilter = L"pwm\0*.pwm\0";
		ofn.nFileExtension = 1;
		ret = GetSaveFileName(&ofn);
	}

	if( ret ) {
		
		if( type == GetFilenameType::save ) {
			size_t cch = wcslen(szFilename);
			wcscpy_s(szFilename + cch, MAX_PATH - cch, L".xml");
		} else if( type == GetFilenameType::saveEnc ) {
			size_t cch = wcslen(szFilename);
			wcscpy_s(szFilename + cch, MAX_PATH - cch, L".pwm");
		}

		*pszFilename = szFilename;

		return true;

	} else {
		error = CommDlgExtendedError();
	}

	delete[] szFilename;
	return false;
}

bool OpenFile(HWND hwndParent, const WCHAR * szFilepath) {

	const WCHAR * szFilename = nullptr;
	if( !szFilepath ) {
		if( !GetFilename(hwndParent, GetFilenameType::open, &szFilename) )
			return false;
	} else {
		szFilename = szFilepath;
	}
		
	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	
	if( pUserData->pAccounts )
		Close(hwndParent);

	pUserData->pAccounts = new Accounts;
	bool ret;
	if( (ret = pUserData->pAccounts->Load(szFilename)) ) {
		UpdateMenuItems(hwndParent);
		UpdateDisplay(hwndParent);
		SetWindowText(hwndParent, szFilename);
		pUserData->pRecentFiles->FileOpened(szFilename);
		//ResetRecentFilesMenu(hwndParent);


		// Create timer
	}	
			
	if( !szFilepath)
		delete[] szFilename;

	return ret;
}

void LaunchBrowser(HWND hwnd) {
	DWORD error;

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	const Account * paccount;
	if( GetSelectedAccount(hwnd, &paccount) ) {;

		if( paccount->getString(Account::Field::URL).size() == 0 ) {
			wstring msg = paccount->getString(Account::Field::NAME) + L" has no URL.";
			MessageBox(hwnd, msg.c_str(), L"No URL", MB_ICONEXCLAMATION);
			return;
		}

		wstring cmdline = pUserData->pBrowserCommand->GetParameters();
		cmdline += L" ";
		cmdline += paccount->getString(Account::Field::URL);
		/*cmdline += paccount->url;
		cmdline += L" ";
		cmdline += paccount->usernamefield + L" " + paccount->username;
		cmdline += L" ";
		cmdline += paccount->passwordfield + L" " + paccount->password;*/

		PROCESS_INFORMATION pi{ 0 };
		STARTUPINFO si{ 0 };
		WCHAR * szCmdLine = new WCHAR[cmdline.size() + 1]{0};
		wcscpy_s(szCmdLine, cmdline.size() + 1, cmdline.c_str());
		/*if( CreateProcess(L"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe", szCmdLine, NULL,
			NULL, FALSE, 0, NULL, NULL, &si, &pi) ) {
			CloseHandle(pi.hProcess);
			CopyToClipboard(hwnd);
		} else {
			error = GetLastError();
		}*/
		error = (int) ShellExecute(nullptr, L"open", pUserData->pBrowserCommand->GetBrowserPath().c_str(),
			szCmdLine, NULL, SW_SHOW);
		if( 32 < error ) {
			//CopyToClipboard(hwnd);
			LaunchClipboardMonitor(hwnd, false);
		}
		delete[] szCmdLine;
	}
}

//void SetIEFocus(IWebBrowser * pWebBrowser, const wstring& fieldid) {
//
//	HRESULT hr;
//
//	CComPtr<IDispatch> pdispDocument;
//	// get_Document may return (0x8001010d, An outgoing call 
//	// cannot be made since the application is dispatching an input-synchronous call.)
//	// In which case, try get_Document again.
//	while( ( hr = pWebBrowser->get_Document(&pdispDocument) ) == 0x8001010d );
//	if( S_OK != hr || !pdispDocument )
//		return;
//
//	CComQIPtr<IHTMLDocument3> pDocument{ pdispDocument };
//	if( !pDocument )
//		return;
//	
//	CComPtr<IHTMLElement> pElement;
//	hr = pDocument->getElementById(CComBSTR{ fieldid.c_str() }, &pElement);
//	if( S_OK != hr || !pElement )
//		return;
//
//	CComQIPtr<IHTMLControlElement> pInputElement{ pElement };
//	if( !pInputElement )
//		return;
//
//	hr = pInputElement->focus();
//}

void ShowClipboardMonitorDialogEx(HWND hwndParent, UserData * pUserData, const Account * paccount, 
	const std::vector<ClipboardMonitor::CM_string_pair> * pStrs) {

	if( !paccount )	return;

	HINSTANCE hInstance = (HINSTANCE) GetModuleHandle(nullptr);

	auto hwnd = CreateDialogParam(hInstance,
		MAKEINTRESOURCE(IDD_DIALOG_CB),
		hwndParent,
		ClipboardMonitor::MainDlgProc,
		(LPARAM) pStrs);

	pUserData->hwndClipboardMonitor = hwnd;

	RECT appRect, diagRect;
	GetWindowRect(hwndParent, &appRect);
	GetWindowRect(hwnd, &diagRect);

	int diagCY = diagRect.bottom - diagRect.top;

	int x = 5 + appRect.left;
	int y = appRect.bottom - 5 - diagCY;

	SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

}

void ShowClipboardMonitorDialog(HWND hwndParent, UserData * pUserData, const Account * paccount, bool fIncludeURL) {

	std::vector<ClipboardMonitor::CM_string_pair> *	pPairs = 
		new std::vector<ClipboardMonitor::CM_string_pair>{};
	if( !pPairs )	return;
	
	if( fIncludeURL )
		pPairs->push_back({ paccount->getString(Account::Field::URL), false });

	pPairs->push_back({ paccount->getString(Account::Field::USERNAME), false });
	pPairs->push_back({ paccount->getString(Account::Field::PASSWORD), true });
	
	ShowClipboardMonitorDialogEx(hwndParent, pUserData, paccount, pPairs);
}

//IWebBrowser2 * LaunchIE(const WCHAR * szURL) {
//
//	CLSID clsidIE;
//	HRESULT hr = CLSIDFromProgID(L"InternetExplorer.Application", &clsidIE);
//
//	IWebBrowser2 * pBrowser = nullptr;
//	hr = CoCreateInstance(clsidIE, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pBrowser));
//	if( S_OK == hr && pBrowser ) {
//		pBrowser->put_Visible(VARIANT_TRUE);
//		HWND hwndIE;
//		hr = pBrowser->get_HWND((SHANDLE_PTR *) &hwndIE);
//		SetForegroundWindow(hwndIE);
//		VARIANT varEmpty{ 0 }; varEmpty.vt = VT_ERROR;
//		auto bstrUrl = SysAllocString(szURL);
//		hr = pBrowser->Navigate(bstrUrl, &varEmpty, &varEmpty, &varEmpty, &varEmpty);
//	}
//
//	return pBrowser;
//}

void LaunchClipboardMonitor(HWND hwnd, bool fIncludeURL) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !pUserData )
		return;

	if( pUserData->hwndClipboardMonitor ) {
		SendMessage(pUserData->hwndClipboardMonitor, WM_CLOSE, NULL, NULL);
		pUserData->hwndClipboardMonitor = NULL;
	}
	
	const Account * paccount;
	if( GetSelectedAccount(hwnd, &paccount) ) {

		ShowClipboardMonitorDialog(hwnd, pUserData, paccount, fIncludeURL);
	}
}

void LaunchClipboardMonitorWithAccount(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !pUserData )
		return;

	if( pUserData->hwndClipboardMonitor ) {
		SendMessage(pUserData->hwndClipboardMonitor, WM_CLOSE, NULL, NULL);
		pUserData->hwndClipboardMonitor = NULL;
	}

	const Account * paccount;
	if( GetSelectedAccount(hwnd, &paccount) ) {

		std::unique_ptr<std::vector<std::wstring>> pStrs{ paccount->getStrings(Account::Field::ALL) };
		if( !pStrs.get() )	return;

		auto str_password = paccount->getString(Account::Field::PASSWORD);

		std::vector<ClipboardMonitor::CM_string_pair> * pPairs =
			new std::vector<ClipboardMonitor::CM_string_pair>{};

		for( auto& str : *pStrs ) {
			auto fPassword = str == str_password;
			pPairs->push_back({ std::move(str), fPassword });
		}
		
		ShowClipboardMonitorDialogEx(hwnd, pUserData, paccount, pPairs);

	}
}

//void LaunchBrowserClipboard(HWND hwnd) {
//
//	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
//	if( !pUserData )
//		return;
//
//	const Account * paccount;
//	if( GetSelectedAccount(hwnd, &paccount) ) {
//
//		IWebBrowser2 * pBrowser = LaunchIE(paccount->url.c_str());
//		pUserData->pWebBrowser = (void *) pBrowser;
//		CopyToClipboard(hwnd);
//		pBrowser->Release();
//		pUserData->pWebBrowser = nullptr;
//
//	}
//
//}

void LaunchAgnosticBrowserClipboard(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !pUserData )
		return;

	const Account * paccount;
	if( GetSelectedAccount(hwnd, &paccount) ) {

		if( paccount->getString(Account::Field::URL).size() == 0 ) {
			wstring msg = paccount->getString(Account::Field::NAME) + L" has no URL.";
			MessageBox(hwnd, msg.c_str(), L"No URL", MB_ICONEXCLAMATION);
			return;
		}

		/*auto ret = (int) ShellExecute(nullptr, L"open", paccount->url.c_str(),
			nullptr, nullptr, SW_SHOW);*/

		std::wstring exec = pUserData->pBrowserCommand->GetDefaultBrowserPath();

		std::wstring params;
		params += pUserData->pBrowserCommand->GetDefaultParameters();
		params += L" ";
		params += paccount->getString(Account::Field::URL).c_str();

		auto ret = (int) ShellExecute(nullptr, L"open", exec.c_str(),
			params.c_str(), nullptr, SW_SHOW);
		if( ret > 32 ) {
			//CopyToClipboard(hwnd);
			LaunchClipboardMonitor(hwnd, false);
		}
		
	}

}

void UpdateMenuItems(HWND hwnd) {
	static bool fFirstRun = true;

	HMENU hMenu = GetMenu(hwnd);

	BOOL ret;
	auto cursel = ListBox_GetCurSel(hwnd);
	if( cursel != LB_ERR && !fFirstRun ) {
		// Action items to enable if there IS a current selection
		ret = EnableMenuItem(hMenu, ID_ACTIONS_LAUNCHBROWSER, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_LAUNCHBROWSERCB, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_COPY, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_COPY_URL, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_EXPORTXML, MF_ENABLED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_EDITENTRY, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_REMOVEENTRY, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_CLONEENTRY, MF_ENABLED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEUP, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEDOWN, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEAFTER, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEBEFORE, MF_ENABLED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_COPY_ACCOUNT, MF_ENABLED);

	} else {

		ret = EnableMenuItem(hMenu, ID_ACTIONS_LAUNCHBROWSER, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_LAUNCHBROWSERCB, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_COPY, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_COPY_URL, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_EXPORTXML, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_EDITENTRY, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_REMOVEENTRY, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_CLONEENTRY, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEUP, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEDOWN, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEAFTER, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEBEFORE, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_COPY_ACCOUNT, MF_GRAYED);
		
		fFirstRun = false;
	}

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( pUserData->pAccounts ) {

		if( pUserData->pAccounts->IsBackedByFile() )
			ret = EnableMenuItem(hMenu, ID_FILE_SAVE, MF_ENABLED);
		else 
			ret = EnableMenuItem(hMenu, ID_FILE_SAVE, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_FILE_SAVEAS, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_FILE_SAVEENCRYPTED, MF_ENABLED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_ADDENTRY, MF_ENABLED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_IMPORTXML, MF_ENABLED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_SEARCHENTRY, MF_ENABLED);

	} else {

		ret = EnableMenuItem(hMenu, ID_FILE_SAVE, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_FILE_CLOSE, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_FILE_SAVEAS, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_FILE_SAVEENCRYPTED, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_ADDENTRY, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_IMPORTXML, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_SEARCHENTRY, MF_GRAYED);
	}
}

bool GetSelectedAccount(HWND hwndListBox, const Account ** ppAccount) {
	
	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndListBox, GWLP_USERDATA);

	if( !pUserData->pAccounts )	return false;

	const std::vector<Account>& accounts = pUserData->pAccounts->GetAccounts();

	long cursel = ListBox_GetCurSel(hwndListBox);
	if( cursel > -1 && cursel < accounts.size() ) {
		const Account& account = accounts.at(cursel);
		*ppAccount = &account;
		return true;
	}

	return false;
}

long GetSelectedAccountIndex(HWND hwndListBox) {

	/*UserData * pUserData = (UserData *) GetWindowLongPtr(hwndListBox, GWLP_USERDATA);
	const std::vector<Account>& accounts = pUserData->pAccounts->GetAccounts();*/
	long cursel = ListBox_GetCurSel(hwndListBox);
	/*if( cursel > -1 && cursel < accounts.size() )
		return cursel;*/

	/*return -1;*/
	return cursel;
}

Account * LaunchAccountEditor(HWND hwndParent, const Account * paccount) {
	
	HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndParent, GWLP_HINSTANCE);
	Account * preturnaccount = (Account *) DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_ACCOUNT),
		hwndParent, AccountEditor::MainDlgProc, (LPARAM) paccount);

	return preturnaccount;
}

void EditEntry(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	
	const Account * paccount;
	long index = GetSelectedAccountIndex(hwnd);
	if( GetSelectedAccount(hwnd, &paccount) ) {
		auto peditedaccount = LaunchAccountEditor(hwnd, paccount);
		/*if( (int) peditedaccount == LAUNCH_EDITOR_NEXT ) {
			if( index < pUserData->pAccounts->GetAccounts().size() - 1 ) {
				++index;
				ListBox_SetCurSel(hwnd, index);
				EditEntry(hwnd);
			}
			return;
		} else if( (int) peditedaccount == LAUNCH_EDITOR_PRIOR ) {
			if( index > 0 ) {
				--index;
				ListBox_SetCurSel(hwnd, index);
				EditEntry(hwnd);
			}
			return;
		} else */if( peditedaccount == nullptr ) {
			return;
		}
		
		index = GetSelectedAccountIndex(hwnd);	//	Selection may have changed. PgUp/PgDown keys.
		pUserData->pAccounts->ReplaceAccount(index, *peditedaccount);
		delete peditedaccount;
	}

	UpdateDisplay(hwnd);
}

void AddEntry(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !pUserData->pAccounts )
		return;
	
	auto pnewaccount = LaunchAccountEditor(hwnd, nullptr);
	if( pnewaccount ) {		
		pUserData->pAccounts->AddAccount(std::move(*pnewaccount));
		delete pnewaccount;
	}

	UpdateDisplay(hwnd);
}

void RemoveEntry(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !pUserData->pAccounts )
		return;

	long index = GetSelectedAccountIndex(hwnd);
	if( index == LB_ERR || index > pUserData->pAccounts->GetAccounts().size() )
		return;

	pUserData->pAccounts->RemoveAccount(index);

	UpdateDisplay(hwnd);
}

void Close(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( pUserData->pAccounts ) {
		delete pUserData->pAccounts;
		pUserData->pAccounts = nullptr;
	}

	UpdateDisplay(hwnd);
	UpdateMenuItems(hwnd);
	//ResetRecentFilesMenu(hwnd);

	SetWindowText(hwnd, L"Password Manager");

	// Delete Timer
}

void NewFile(HWND hwndParent) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	if( pUserData->pAccounts )
		Close(hwndParent);

	pUserData->pAccounts = new Accounts;

	UpdateMenuItems(hwndParent);

	SetWindowText(hwndParent, L"New Password Data");

	auto hwndCtl = GetDlgItem(hwndParent, ID_ACTIONS_ADDENTRY);
	auto wparam = MAKEWPARAM(ID_ACTIONS_ADDENTRY, 0);
	PostMessage(hwndParent, WM_COMMAND, wparam, (LPARAM) hwndCtl);
}

void SaveFile(HWND hwndParent) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	if( pUserData->pAccounts ) {
		
		if( pUserData->pAccounts->SaveAccounts() ) {
			auto szFilepath = pUserData->pAccounts->GetFilepath().c_str();
			SetWindowText(hwndParent, szFilepath);
			pUserData->pRecentFiles->FileOpened(szFilepath);
		}

	}

}

void SaveFileEncrypted(HWND hwndParent) {
	
	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	if( pUserData->pAccounts ) {

		const WCHAR * szFilename;
		if( GetFilename(hwndParent, GetFilenameType::saveEnc, &szFilename) ) {

			if( pUserData->pAccounts->SaveAccountsEncrypted(szFilename) ) {

				SetWindowText(hwndParent, szFilename);
				pUserData->pRecentFiles->FileOpened(szFilename);

				UpdateMenuItems(hwndParent);

			} else {
				MessageBox(hwndParent, L"Error saving encrypted file.", L"Password Manager", MB_ICONERROR);
			}

			delete[] szFilename;
		}
	}
}

void SaveFileAs(HWND hwndParent) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	if( !pUserData->pAccounts )
		return;

	const WCHAR * szFilename;
	if( GetFilename(hwndParent, GetFilenameType::save, &szFilename) ) {
				 
		auto fSaved = pUserData->pAccounts->SaveAccountsAs(szFilename);
		if( !fSaved )	return;

		SetWindowText(hwndParent, szFilename);
		
		pUserData->pRecentFiles->FileOpened(szFilename);

		delete[] szFilename;

		UpdateMenuItems(hwndParent);
	}
}

void CreateRecentFilesMenu(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
	HMENU hRecentFiles = pUserData->pRecentFiles->GetRecentFilesMenu();

	MENUITEMINFO mii{ sizeof(MENUITEMINFO), 0 };
	mii.fMask = MIIM_STRING | MIIM_SUBMENU;
	mii.dwTypeData = L"&Recent Files";
	mii.cch = wcslen(mii.dwTypeData);
	mii.hSubMenu = hRecentFiles;
	BOOL ret = InsertMenuItem(hFileMenu, 6, true, &mii);
}

void ResetRecentFilesMenu(HWND hwnd) {
	HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
	DeleteMenu(hFileMenu, 6, MF_BYPOSITION);

	CreateRecentFilesMenu(hwnd);
}

void OpenRecentFile(HWND hwnd, UINT id) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	auto vRecentFiles = pUserData->pRecentFiles->GetRecentFiles();
	UINT index = id - RECENT_FILE_MENU_ID;

	if( index < vRecentFiles.size() && vRecentFiles.size() > 0 ) {

		try {

			OpenFile(hwnd, vRecentFiles[index].c_str());

		} catch(FILE_NOT_FOUND) {

			wstring msg_txt = L"Do you want to remove the entry from the Recent File list?";
			wstring caption = L"File not found error";
			int res = MessageBox(hwnd, msg_txt.c_str(), caption.c_str(), MB_YESNO);
			if( res == IDYES ) {
				pUserData->pRecentFiles->RemoveRecentFile(index);
			}

		}
	}
}

void ClearRecentFileMenu(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	pUserData->pRecentFiles->ClearRecentFiles();
}

void ProcessCommandLine(HWND hwnd, const WCHAR * szCmdLine) {

	auto cmdline = SanitizeCommandLine(szCmdLine);

 	auto ret = OpenFile(hwnd, cmdline.c_str());

}

void UpdateEditorAccount(HWND hwnd, long direction, const Account ** ppaccount) {

	long cursel = ListBox_GetCurSel(hwnd);

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	const std::vector<Account>& accounts = pUserData->pAccounts->GetAccounts();

	long newsel = -1;
	if( direction == GET_NEXT_ACCOUNT ) {

		long nextsel = ++cursel;
		if( nextsel < accounts.size() )
			newsel = nextsel;			

	} else if( direction == GET_PRIOR_ACCOUNT ) {

		long priorsel = --cursel;
		if( priorsel > -1 )
			newsel = priorsel;

	}

	if( newsel > -1 ) {

		const Account& account = accounts.at(newsel);
		*ppaccount = &account;

		ListBox_SetCurSel(hwnd, newsel);

	} else {

		*ppaccount = nullptr;
	}

}

void ShowBrowserCommandDialog(HWND hwndParent) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndParent, GWLP_HINSTANCE);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_BROWSER_CMD), hwndParent,
		BrowserCommandDialog::MainDlgProc, (LPARAM) pUserData->pBrowserCommand);

}

void ShowHelpDialog(HWND hwndParent) {

	DialogBox((HINSTANCE) GetWindowLongPtr(hwndParent, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG_ABOUT),
		hwndParent, AboutDialog::AboutDlgProc);

}

void ShowToolsDialog(HWND hwndParent) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	pUserData->pTools->ShowDialogBox(hwndParent);

	PopulateToolsMenu(hwndParent);
}

void ShowSearchDialog(HWND hwndParent) {

	SearchDialog srch_dlg;

	auto ret = srch_dlg.ShowDialogBox(hwndParent);

	if( ret > -1 ) {
		const Account * paccount = (const Account *) ret;
		ListBox_SetCurSel(hwndParent, paccount->index);
		UpdateMenuItems(hwndParent);
	}
}

void PopulateToolsMenu(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	HMENU hToolsMenu = GetSubMenu(GetMenu(hwnd), 2);

	int count = GetMenuItemCount(hToolsMenu);

	for( int i = 0; i < count - 2; ++i ) {
		DeleteMenu(hToolsMenu, 0, MF_BYPOSITION);
	}

	vector<wstring> captions = pUserData->pTools->GetCaptions();

	int menuID = ID_TOOLS_TOOL1;
	BOOL ret;
	for ( const wstring& caption : captions ) {

		MENUITEMINFO mii{ sizeof(MENUITEMINFO), 0 };
		mii.fMask = MIIM_STRING | MIIM_ID;
		mii.wID = menuID++;
		mii.dwTypeData = (LPWSTR) caption.c_str();
		mii.cch = caption.size();
		ret = InsertMenuItem(hToolsMenu, 0, TRUE, &mii);

	}

}

void LaunchTool(HWND hwndParent, int id) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	int index = id - ID_TOOLS_TOOL1;

	const wstring& target = pUserData->pTools->GetTarget(index);

	wstring wd = GetParentFromFilepath(target);
	const WCHAR * pwd = wd.size() ? wd.c_str() : nullptr;

	auto ret = (int) ShellExecute(nullptr, L"open", target.c_str(),
		nullptr, pwd, SW_SHOW);

}

void ProcessClipboardMonitorClosing(HWND hwndParent) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	if( pUserData ) pUserData->hwndClipboardMonitor = NULL;

}

void ExportXML(HWND hwndParent) {

	const Account * pAccount = nullptr;
	if( GetSelectedAccount(hwndParent, &pAccount) && pAccount ) {

		auto xml = pAccount->to_xml();
		AccountXMLDialog dlg{ xml };
		dlg.ShowDialogBox(hwndParent);

	}

}

void ImportXML(HWND hwndParent) {

	const WCHAR * szImportFilename = nullptr;
	if( GetFilename(hwndParent, GetFilenameType::open, &szImportFilename) ) {

		Accounts imported_accounts;
		if( imported_accounts.Load(szImportFilename) ) {

			UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
			pUserData->pAccounts->AddAccounts(std::move(imported_accounts));

			UpdateDisplay(hwndParent);
		}
	}
}

void SwapAccount(HWND hwndParent, int delta) {

	auto index = GetSelectedAccountIndex(hwndParent);
	if( index < 0 )
		return;

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	int swappedIndex;
	if( pUserData ) 
		swappedIndex = pUserData->pAccounts->SwapAccounts(index, index + delta);

	if( swappedIndex >= 0 ) {
		UpdateDisplay(hwndParent);
		ListBox_SetCurSel(hwndParent, swappedIndex);
	}
}

void MoveAccount(HWND hwndParent, int type) {

	SearchDialog srch_dlg;

	auto ret = srch_dlg.ShowDialogBox(hwndParent);

	if( ret > -1 ) {

		const Account * paccount = (const Account *) ret;

		auto index = GetSelectedAccountIndex(hwndParent);
		if( index < 0 )
			return;

		int dstIndex = paccount->index + type;
		dstIndex = max(0, dstIndex);

		UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);
		int movedIndex;
		if( pUserData )
			movedIndex = pUserData->pAccounts->MoveAccount(index, dstIndex);

		if( movedIndex >= 0 ) {
			UpdateDisplay(hwndParent);
			ListBox_SetCurSel(hwndParent, movedIndex);
		}
	}
}

HMENU CreateContextMenu(HWND hwnd) {
	
	auto hmenuAction = GetSubMenu(GetMenu(hwnd), 1);
	
	return hmenuAction;
}

void ShowContextMenu(HWND hwnd, int clientX, int clientY) {

	if( GetSelectedAccountIndex(hwnd) < 0 )	return;

	const UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	POINT p{ clientX, clientY };
	ClientToScreen(hwnd, &p);

	auto ret = TrackPopupMenuEx(pUserData->hmenuContext, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, hwnd, NULL);
}

void CloneEntry(HWND hwnd) {

	UserData* pUserData = (UserData*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !pUserData->pAccounts )
		return;

	const Account* paccount;
	
	if( GetSelectedAccount(hwnd, &paccount) ) {
		
		auto new_account = paccount->clone();

		auto id = ::GenerateRandomString(8);
		new_account.setString(Account::ID, id);

		pUserData->pAccounts->AddAccount(std::move(new_account));

		UpdateDisplay(hwnd);

		auto size = ListBox_GetCount(hwnd) - 1;
		ListBox_SetCurSel(hwnd, size);
	}

}

void QueryOpenFile(const wchar_t* szPath, bool* fRet) {

	*fRet = false;

	if (szPath != nullptr) {

		std::wstring path{ szPath };

		auto dot = path.find_last_of(L'.');

		auto ext = path.substr(dot + 1, path.length() - dot);
		if (ext == L"xml" ||
			ext == L"pwm")

			*fRet = true;		
	}
}

void PopulateSettingsMenu(HWND hwnd) {
	
	UserData* pUserData = ( UserData* ) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
	HMENU hSettingsMenu = GetSubMenu(hFileMenu, 7);
	
	MENUITEMINFO mii{ sizeof(MENUITEMINFO), 0 };
	mii.fMask = MIIM_STATE;

	if( !pUserData->p_pswdgen_Application ) {

		mii.fState = MFS_GRAYED;

		auto ret = SetMenuItemInfo(hSettingsMenu, 2, TRUE, &mii);
		ret = SetMenuItemInfo(hSettingsMenu, 3, TRUE, &mii);

		return;
	}	
	
	auto fUsePswdGen = pUserData->pSettings->getDWORD(L"fUsePasswordGenerator");
	if( fUsePswdGen ) {
		mii.fState = MFS_CHECKED;
		auto ret = SetMenuItemInfo(hSettingsMenu, 2, TRUE, &mii);
		mii.fState = MFS_ENABLED;
		ret = SetMenuItemInfo(hSettingsMenu, 3, TRUE, &mii);
	} else {
		mii.fState = MFS_ENABLED;
		auto ret = SetMenuItemInfo(hSettingsMenu, 2, TRUE, &mii);
		mii.fState = MFS_GRAYED;
		ret = SetMenuItemInfo(hSettingsMenu, 3, TRUE, &mii);
	}
}

void ToggleUsePasswordGenerator(HWND hwnd) {

	UserData* pUserData = ( UserData* ) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
	HMENU hSettingsMenu = GetSubMenu(hFileMenu, 7);

	MENUITEMINFO mii{ sizeof(MENUITEMINFO), 0 };
	mii.fMask = MIIM_STATE;

	auto ret = GetMenuItemInfo(hSettingsMenu, 2, TRUE, &mii);
	if( mii.fState & MFS_CHECKED ) {
		pUserData->pSettings->setDWORD(L"fUsePasswordGenerator", 0);
	} else {
		pUserData->pSettings->setDWORD(L"fUsePasswordGenerator", 1);
	}

	PopulateSettingsMenu(hwnd);
}

void ShowPasswordGeneratorSettings(HWND hwnd) {

	UserData* pUserData = ( UserData* ) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	IApplication* p_app = pUserData->p_pswdgen_Application.p;

	CComBSTR bstrName{ L"Mode" };
	CComBSTR bstrValue{ L"Settings" };

	VARIANT_BOOL ret;

	HRESULT hr = p_app->SetProperty(bstrName, bstrValue, &ret);

	hr = p_app->put_Visible(VARIANT_TRUE);
}