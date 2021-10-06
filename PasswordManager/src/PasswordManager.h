#pragma once

#include "Accounts.h"
#include "RecentFiles.h"
#include "BrowserCommandDialog.h"
#include "Tools.h"
#include "ApplicationSettings.h"

#define APPLICATION_SUBKEY	L"SOFTWARE\\Clifford\\PasswordManager"

#define CBM_CLOSING (WM_USER + 10)

#define PM_QUERYOPENFILE (WM_APP + 1)
#define PM_DRAGDROPOPENFILE (WM_APP + 2)

struct UserData {
	LONG_PTR				oldproc = 0x00;
	Accounts				* pAccounts = nullptr;
	//void 					* pWebBrowser = nullptr;
	RecentFiles				* pRecentFiles = nullptr;
	BrowserCommand			* pBrowserCommand = nullptr;
	Tools					* pTools = nullptr;
	ApplicationSettings		* pSettings = nullptr;

	HWND					hwndClipboardMonitor;

	HMENU					hmenuContext = NULL;

	WCHAR					* szTempFilePath = nullptr;

	~UserData() {
		if( pAccounts )			delete pAccounts;
		//if( pWebBrowser )		((IUnknown *) pWebBrowser)->Release();
		if( pRecentFiles )		delete pRecentFiles;
		if( pBrowserCommand )	delete pBrowserCommand;
		if( szTempFilePath )	delete[] szTempFilePath;
		if( pTools )			delete pTools;
		if( pSettings )			delete pSettings;
	}
};

extern UserData* g_pUserData;	// PasswordManager.cpp