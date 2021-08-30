#include <Windows.h>
#include <windowsx.h>
//#include <MsXml6.h>

#include "resource.h"
#include "Accounts.h"
#include "AccountEditor.h"
#include "RecentFiles.h"
#include "BrowserCommandDialog.h"
#include "App.h"
#include "Tools.h"
#include "PasswordManager.h"

#define ComCtl6
#include "CommonHeaders.h"

#pragma comment(lib, "Comdlg32.lib")

LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void Cls_OnDestroy(HWND hwnd);
void Cls_OnTimer(HWND hwnd, UINT id);
void Cls_OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys);
void Cls_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void Cls_OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void Cls_OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void Cls_OnSize(HWND hwnd, UINT state, int cx, int cy);

BOOL Cls_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);

LONG_PTR RegisterMainWindow() {

	WNDCLASS wndclass;
	GetClassInfo(NULL, L"ListBox", &wndclass);

	wndclass.lpszClassName = L"ListBoxCustom";

	WNDPROC oldproc = wndclass.lpfnWndProc;
	wndclass.lpfnWndProc = MainWndProc;

	auto ret = RegisterClass(&wndclass);

	return (LONG_PTR) oldproc;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, WCHAR * szCmdLine, int iShowCmd) {
	
	HMENU	hmenuMain = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	HACCEL	haccelerators = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	UserData	* pUserData = new UserData;
	pUserData->oldproc = RegisterMainWindow();
	pUserData->pSettings = new ApplicationSettings{ APPLICATION_SUBKEY };

	auto x = pUserData->pSettings->getDWORD(L"x");				if( 0 == x )	x = 100;
	auto y = pUserData->pSettings->getDWORD(L"y");				if( 0 == y )	y = 100;
	auto width = pUserData->pSettings->getDWORD(L"width");		if( 0 == width )	width = 800;
	auto height = pUserData->pSettings->getDWORD(L"height");	if( 0 == height )	height = 600;

	auto fMaximized = pUserData->pSettings->getDWORD(L"fMaximized");

	auto hwnd = CreateWindow(L"ListBoxCustom", L"Password Manager",
		WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL |LBS_USETABSTOPS,
		x, y, width, height,
		NULL, hmenuMain, hInstance, (void *) pUserData);
	if( !hwnd ) {
		DWORD error = GetLastError();
		return error;
	}
	
	if( wcslen(szCmdLine) > 0 )
		ProcessCommandLine(hwnd, szCmdLine);

	UpdateWindow(hwnd);
	ShowWindow(hwnd, fMaximized ?  SW_MAXIMIZE : iShowCmd);

	MSG msg{ 0 };
	while( GetMessage(&msg, NULL, 0, 0) ) {
		if( !TranslateAccelerator(hwnd, haccelerators, &msg) ) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete pUserData;

	return (int) msg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	
	static WNDPROC oldlistboxproc{ nullptr };
	if( !oldlistboxproc ) {
		WNDCLASS wndclass;
		GetClassInfo(NULL, L"ListBox", &wndclass);
		oldlistboxproc = wndclass.lpfnWndProc;
	}

	switch( message ) {
		
		HANDLE_MSG(hwnd, WM_DESTROY, Cls_OnDestroy);
		HANDLE_MSG(hwnd, WM_COMMAND, Cls_OnCommand);		
		HANDLE_MSG(hwnd, WM_TIMER, Cls_OnTimer);
		HANDLE_MSG(hwnd, WM_MOUSEWHEEL, Cls_OnMouseWheel);
		HANDLE_MSG(hwnd, WM_SIZE, Cls_OnSize);
			
		HANDLE_MSG(hwnd, WM_RBUTTONDOWN, Cls_OnRButtonDown);
		HANDLE_MSG(hwnd, WM_RBUTTONUP, Cls_OnRButtonUp);

	case WM_CREATE:

		Cls_OnCreate(hwnd, (LPCREATESTRUCT) lParam);
		
		break;	// Need to allow the ListBox to run it's WM_CREATE routine.
				// So we don't use the HANDLE_MSG WM_CREATE cracker.


	case WM_KEYDOWN: {

		auto ret = CallWindowProc(oldlistboxproc, hwnd, message, wParam, lParam);
		
		Cls_OnKey(hwnd, (UINT) wParam, TRUE, (int) (short) LOWORD(lParam), (UINT) HIWORD(lParam));

		return ret; }

	//	Received each time the list box selection changes.
	case WM_CAPTURECHANGED:
		UpdateMenuItems(hwnd);
		break;

	case GET_NEXT_ACCOUNT:
	case GET_PRIOR_ACCOUNT:

		UpdateEditorAccount(hwnd, (long) message, (const Account **) lParam);

		break;

	case CBM_CLOSING:
		ProcessClipboardMonitorClosing(hwnd);
		break;

	}

	return CallWindowProc(oldlistboxproc, hwnd, message, wParam, lParam);
}

WCHAR * g_szTempFilePath;
HWND	g_hwndApp;

BOOL Cls_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	
	DWORD error;

	UserData * pUserData  = (UserData *) lpCreateStruct->lpCreateParams;
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pUserData);

	UpdateMenuItems(hwnd);

	pUserData->pRecentFiles = new RecentFiles{ pUserData->pSettings };
	CreateRecentFilesMenu(hwnd);

	pUserData->pBrowserCommand = new BrowserCommand{ pUserData->pSettings };

	pUserData->pTools = new Tools(pUserData->pSettings);

	PopulateToolsMenu(hwnd);

	pUserData->szTempFilePath = new WCHAR[MAX_PATH + 1]{ 0 };
	if( pUserData->szTempFilePath ) {
		auto res = GetTempPath(MAX_PATH, pUserData->szTempFilePath);
		if( res ) {
			wsprintf(pUserData->szTempFilePath + res, L"%s\\", L"PasswordManager");
			g_szTempFilePath = pUserData->szTempFilePath;
			auto ret = CreateDirectory(pUserData->szTempFilePath, nullptr);
			
			if( !ret ) error = GetLastError();
		}
	}

	g_hwndApp = hwnd;

	pUserData->hmenuContext = CreateContextMenu(hwnd);
	
	return TRUE;
}

void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	
	switch( id ) {

		case ID_ACC_EXIT:
		case ID_FILE_EXIT:
			ExitApplication(hwnd);
			break;

		case ID_ACC_OPEN:
		case ID_FILE_OPEN:
			OpenFile(hwnd, nullptr);
			break;

		case ID_ACC_NEW:
		case ID_FILE_NEW:
			NewFile(hwnd);
			break;

		case ID_ACC_SAVE:
		case ID_FILE_SAVE:
			SaveFile(hwnd);
			break;

		case ID_ACC_SAVEAS:
		case ID_FILE_SAVEAS:
			SaveFileAs(hwnd);
			break;

		case ID_ACC_SAVEENCRYPTED:
		case ID_FILE_SAVEENCRYPTED:
			SaveFileEncrypted(hwnd);
			break;

		case ID_FILE_CLOSE:
			Close(hwnd);
			break;

		case ID_SETTINGS_BROWSERCMD:
			ShowBrowserCommandDialog(hwnd);
			break;

		case ID_ACC_LAUNCHBROWSER:
		case ID_ACTIONS_LAUNCHBROWSER:
			LaunchBrowser(hwnd);
			break;
		
		case ID_ACC_LAUNCHBROWSERCB:
		case ID_ACTIONS_LAUNCHBROWSERCB:
			//LaunchBrowserClipboard(hwnd);
			LaunchAgnosticBrowserClipboard(hwnd);
			break;

		case ID_ACC_COPY:
		case ID_ACTIONS_COPY:
			LaunchClipboardMonitor(hwnd, false);
			break;

		case ID_ACC_COPY_URL:
		case ID_ACTIONS_COPY_URL:
			LaunchClipboardMonitor(hwnd, true);
			break;

		case ID_ACC_COPY_ACCOUNT:
		case ID_ACTIONS_COPY_ACCOUNT:
			LaunchClipboardMonitorWithAccount(hwnd);
			break;

		case ID_ACC_EXPORTXML:
		case ID_ACTIONS_EXPORTXML:
			ExportXML(hwnd);
			break;

		case ID_ACC_IMPORTXML:
		case ID_ACTIONS_IMPORTXML:
			ImportXML(hwnd);
			break;

		case ID_ACC_EDIT:
		case ID_ACTIONS_EDITENTRY:
			EditEntry(hwnd);
			break;

		case ID_ACC_ADD:
		case ID_ACTIONS_ADDENTRY:
			AddEntry(hwnd);
			break;		

		case ID_ACC_REMOVE:
		case ID_ACTIONS_REMOVEENTRY:
			RemoveEntry(hwnd);
			break;

		case ID_HELP_ABOUT:
			ShowHelpDialog(hwnd);
			break;

		case ID_TOOLS_EDITTOOLS:
			ShowToolsDialog(hwnd);
			break;

		case ID_ACC_SEARCH:
		case ID_ACTIONS_SEARCHENTRY:
			ShowSearchDialog(hwnd);

			break;

		case ID_ACC_MOVEUP:
		case ID_ACTIONS_MOVEUP:
			SwapAccount(hwnd, -1);
			break;

		case ID_ACC_MOVEDOWN:
		case ID_ACTIONS_MOVEDOWN:
			SwapAccount(hwnd, 1);
			break;

		case ID_ACC_MOVEBEFORE:
		case ID_ACTIONS_MOVEBEFORE:
			MoveAccount(hwnd, MOVE_ACCOUNT_BEFORE);
			break;

		case ID_ACC_MOVEAFTER:
		case ID_ACTIONS_MOVEAFTER:
			MoveAccount(hwnd, MOVE_ACCOUNT_AFTER);
			break;

		case ID_ACC_CLONE_ACCOUNT:
		case ID_ACTIONS_CLONEENTRY:
			CloneEntry(hwnd);
			break;
	}

	if( id >= RECENT_FILE_MENU_ID && id < RECENT_FILE_MENU_ID + 5 ) {
		OpenRecentFile(hwnd, id);
		return;
	}

	if( id == RECENT_FILE_MENU_CLEAR ) {
		ClearRecentFileMenu(hwnd);
		return;
	}

	if( id >= ID_TOOLS_TOOL1 ) {
		LaunchTool(hwnd, id);
		return;
	}
}

void Cls_OnDestroy(HWND hwnd) {

	ExitApplication(hwnd);

}

void Cls_OnTimer(HWND hwnd, UINT id) {


}

void Cls_OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys) {

	auto ctrl_state = GetKeyState(VK_CONTROL);
	auto shift_state = GetKeyState(VK_SHIFT);

	static const UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if( ctrl_state & 1 << 15 ) {

		auto cur_sel = ListBox_GetCurSel(hwnd);
		auto size = ListBox_GetCount(hwnd);

		int delta = zDelta / 120;
		if( shift_state & 1 << 15 )	delta *= 5;

		cur_sel -= delta;

		cur_sel = max(0, min(size - 1, cur_sel));

		ListBox_SetCurSel(hwnd, cur_sel);

	} else {

		CallWindowProc((WNDPROC) (pUserData->oldproc), hwnd, WM_MOUSEWHEEL, MAKEWPARAM(fwKeys, zDelta), MAKELPARAM(xPos, yPos));

	}
}

void Cls_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags) {

	auto cur_sel = ListBox_GetCurSel(hwnd);

	if( cur_sel > -1 )
		UpdateMenuItems(hwnd);

	switch( vk ) {

	case VK_APPS:
		ShowContextMenu(hwnd, 10, 10);
		break;
	}
}

void Cls_OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) {

	static const UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if( !pUserData )	return;

	CallWindowProc((WNDPROC) pUserData->oldproc, hwnd, WM_LBUTTONDOWN, (WPARAM) keyFlags, MAKELPARAM(x, y));

	ShowContextMenu(hwnd, x, y);
}

void Cls_OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags) {

	static const UserData * pUserData = (UserData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if( !pUserData )	return;

	CallWindowProc((WNDPROC) pUserData->oldproc, hwnd, WM_LBUTTONUP, (WPARAM) keyFlags, MAKELPARAM(x, y));
}

void Cls_OnSize(HWND hwnd, UINT state, int cx, int cy) {

	UserData* pUserData = (UserData*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( !pUserData )	return;

	switch( state ) {

	case SIZE_MAXIMIZED:
		pUserData->pSettings->setDWORD(L"fMaximized", 1);
		break;

	case SIZE_RESTORED: 
		pUserData->pSettings->setDWORD(L"fMaximized", 0);
		break;

	}

}
