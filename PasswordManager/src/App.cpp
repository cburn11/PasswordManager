#include <Windows.h>
#include <windowsx.h>
#include <commdlg.h>
//#include <MsXml6.h>
//#include <ExDisp.h>
//#include <MsHTML.h>
#include <ShlObj.h>

#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <memory>

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

#define UPDATE_SELECTED_ACCOUNT 0xE001

using std::wstring;
using std::to_wstring;

using std::for_each;
using std::end;
using std::begin;

using std::vector;

bool GetSelectedAccount(HWND hwndListBox, const Account ** ppAcount);

void UpdateDisplay(HWND hwnd) {

	ListBox_ResetContent(hwnd);

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !( pUserData->pAccounts ) )
		return;

	const vector<Account>& accounts = pUserData->pAccounts->GetAccounts();

	auto DisplayAccount = [hwnd](const Account& account) {

		ListBox_AddString(hwnd, account.to_string().c_str());
	};

	for_each(begin(accounts), end(accounts), DisplayAccount);

}

void ExitApplication(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( pUserData->pSettings ) {
		
		RECT rect;
		if( GetWindowRect(hwnd, &rect) ) {

			if( rect.left > 0 && rect.top > 0 ) {

				pUserData->pSettings->height = rect.bottom - rect.top;
				pUserData->pSettings->width = rect.right - rect.left;
				pUserData->pSettings->x = rect.left;
				pUserData->pSettings->y = rect.top;
			}
		}
	}

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
		ResetRecentFilesMenu(hwndParent);


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

		if( paccount->url.size() == 0 ) {
			wstring msg = paccount->name + L" has no URL.";
			MessageBox(hwnd, msg.c_str(), L"No URL", MB_ICONEXCLAMATION);
			return;
		}

		wstring cmdline = pUserData->pBrowserCommand->GetParameters();
		cmdline += L" ";
		cmdline += paccount->url;
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
			LaunchClipboardMonitor(hwnd);
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

void ShowClipboardMonitorDialog(HWND hwndParent, UserData * pUserData, const Account * paccount) {

	HINSTANCE hInstance = (HINSTANCE) GetModuleHandle(nullptr);

	auto hwnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CB), hwndParent,
		ClipboardMonitor::MainDlgProc, (LPARAM) paccount);

	pUserData->hwndClipboardMonitor = hwnd;

	RECT appRect, diagRect;
	GetWindowRect(hwndParent, &appRect);
	GetWindowRect(hwnd, &diagRect);

	int diagCY = diagRect.bottom - diagRect.top;

	int x = 5 + appRect.left;
	int y = appRect.bottom - 5 - diagCY;

	SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

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

void LaunchClipboardMonitor(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !pUserData )
		return;

	if( pUserData->hwndClipboardMonitor ) {
		SendMessage(pUserData->hwndClipboardMonitor, WM_CLOSE, NULL, NULL);
		pUserData->hwndClipboardMonitor = NULL;
	}
	
	const Account * paccount;
	if( GetSelectedAccount(hwnd, &paccount) ) {

		ShowClipboardMonitorDialog(hwnd, pUserData, paccount);
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

		if( paccount->url.size() == 0 ) {
			wstring msg = paccount->name + L" has no URL.";
			MessageBox(hwnd, msg.c_str(), L"No URL", MB_ICONEXCLAMATION);
			return;
		}

		auto ret = (int) ShellExecute(nullptr, L"open", paccount->url.c_str(),
			nullptr, nullptr, SW_SHOW);
		if( ret > 32 ) {
			//CopyToClipboard(hwnd);
			LaunchClipboardMonitor(hwnd);
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
		ret = EnableMenuItem(hMenu, ID_ACTIONS_EXPORTXML, MF_ENABLED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_EDITENTRY, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_REMOVEENTRY, MF_ENABLED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEUP, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEDOWN, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEAFTER, MF_ENABLED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEBEFORE, MF_ENABLED);

	} else {

		ret = EnableMenuItem(hMenu, ID_ACTIONS_LAUNCHBROWSER, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_LAUNCHBROWSERCB, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_COPY, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_EXPORTXML, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_EDITENTRY, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_REMOVEENTRY, MF_GRAYED);

		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEUP, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEDOWN, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEAFTER, MF_GRAYED);
		ret = EnableMenuItem(hMenu, ID_ACTIONS_MOVEBEFORE, MF_GRAYED);

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
			ResetRecentFilesMenu(hwndParent);
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
				ResetRecentFilesMenu(hwndParent);

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
				 
		pUserData->pAccounts->SaveAccountsAs(szFilename);
		SetWindowText(hwndParent, szFilename);
		
		pUserData->pRecentFiles->FileOpened(szFilename);

		delete[] szFilename;

		UpdateMenuItems(hwndParent);
		ResetRecentFilesMenu(hwndParent);
	}

	
}

void CreateRecentFilesMenu(HWND hwnd) {
	HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
	HMENU hRecentFiles = CreatePopupMenu();

	MENUITEMINFO mii{ sizeof(MENUITEMINFO), 0 };
	mii.fMask = MIIM_STRING | MIIM_SUBMENU;
	mii.dwTypeData = L"&Recent Files";
	mii.cch = wcslen(mii.dwTypeData);
	mii.hSubMenu = hRecentFiles;
	BOOL ret = InsertMenuItem(hFileMenu, 6, true, &mii);

	UpdateRecentFilesMenu(hwnd);
}

void ResetRecentFilesMenu(HWND hwnd) {
	HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
	DeleteMenu(hFileMenu, 6, MF_BYPOSITION);

	CreateRecentFilesMenu(hwnd);
}

void UpdateRecentFilesMenu(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	auto vRecentFiles = pUserData->pRecentFiles->GetRecentFiles();
	BOOL ret;
	if( vRecentFiles.size() > 0 ) {
		ULONG menuID = RECENT_FILE_MENU_ID;
		HMENU hFileMenu = GetSubMenu(GetMenu(hwnd), 0);
		HMENU hRecentFiles = GetSubMenu(hFileMenu, 6);
		
		MENUITEMINFO mii{ sizeof(MENUITEMINFO), 0 };
		mii.fMask = MIIM_STRING | MIIM_ID;

		int index{ 0 };
		for_each(begin(vRecentFiles), end(vRecentFiles),
			[hRecentFiles, &menuID, &ret, &index, &mii](wstring const & filepath) {

			++index;
			wstring caption = L"&" + to_wstring(index) + L"  " + filepath.c_str();
			mii.dwTypeData = (LPWSTR) caption.c_str();

			mii.wID = menuID++;
			mii.cch = wcslen(caption.c_str());

			ret = InsertMenuItem(hRecentFiles, index - 1, TRUE, &mii);
		});		

		wstring caption{ L"&Clear" };
		mii.wID = RECENT_FILE_MENU_CLEAR;
		mii.dwTypeData = (LPWSTR) caption.c_str();
		mii.cch = wcslen(caption.c_str());

		ret = InsertMenuItem(hRecentFiles, index, TRUE, &mii);
	}
}

void OpenRecentFile(HWND hwnd, UINT id) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	auto vRecentFiles = pUserData->pRecentFiles->GetRecentFiles();
	UINT index = id - RECENT_FILE_MENU_ID;

	if( index < vRecentFiles.size() && vRecentFiles.size() > 0 ) {

		if( !OpenFile(hwnd, vRecentFiles[index].c_str()) ) {

			wstring msg_txt = L"Do you want to remove the entry from the Recent File list?";
			wstring caption = L"Error Opening File";
			int res = MessageBox(hwnd, msg_txt.c_str(), caption.c_str(), MB_YESNO);
			if( res == IDYES ) {
				pUserData->pRecentFiles->RemoveRecentFile(index);
				ResetRecentFilesMenu(hwnd);
			}

		}
	}
}

void ClearRecentFileMenu(HWND hwnd) {

	UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	pUserData->pRecentFiles->ClearRecentFiles();

	ResetRecentFilesMenu(hwnd);
}

wstring SanitizeCommandLine(const WCHAR * szCmdLine) {

	wstring cmdline{ szCmdLine };

	using std::wregex;
	using std::wsmatch;
	using std::regex_search;

	wregex	RegExp{ LR"(([A-Z]|[a-z]|:|\\|\.|\s|\d|-)+)" };
	wsmatch	match;

	auto res = regex_search(cmdline, match, RegExp);
	if( res )	
		return wstring{ match[0].first, match[0].second };
	else
		return L"";

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

	auto ret = (int) ShellExecute(nullptr, L"open", target.c_str(),
		nullptr, nullptr, SW_SHOW);

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

			auto accounts = imported_accounts.GetAccounts();
			UserData * pUserData = (UserData *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);

			for( auto account : accounts ) {
				pUserData->pAccounts->AddAccount(std::move(account));
			}

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