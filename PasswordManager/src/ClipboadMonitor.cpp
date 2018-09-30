#include <Windows.h>
#include <windowsx.h>

#include <string>
using std::wstring;

#include "CommonHeaders.h"

#include "PasswordManager.h"
#include "resource.h"
#include "Accounts.h"
#include "ClipboardMonitor.h"

HWND g_hwnd;
HWND g_hwndList;

Account * g_pAccount = nullptr;

namespace ClipboardMonitor {

	bool GetListBoxSelectedText(WCHAR ** pszText) {

		WCHAR * szText = nullptr;

		int sel = ListBox_GetCurSel(g_hwndList);
		int cch = ListBox_GetTextLen(g_hwndList, sel);

		szText = ( WCHAR * ) new WCHAR[cch + 1]{ 0 };
		*pszText = szText;
		if( !szText ) return false;

		ListBox_GetText(g_hwndList, sel, szText);

		return true;
	}

	void PutStringOnClipboard(const WCHAR * sz) {

		while( !OpenClipboard(NULL) ) {

		}

		EmptyClipboard();

		auto cbsz = ( wcslen(sz) + 1 ) * sizeof(*sz);

		auto hgMem = GlobalAlloc(GMEM_MOVEABLE, cbsz);
		if( hgMem ) {
			BYTE * pdata = (BYTE *) GlobalLock(hgMem);
			memcpy(pdata, sz, cbsz);
			GlobalUnlock(hgMem);
			HANDLE hdata = SetClipboardData(CF_UNICODETEXT, hgMem);
			DWORD error;
			if( NULL == hdata ) {
				error = GetLastError();
#ifdef _DEBUG
			} else {
				wstring output = sz;
				output += L" put on clipboard.\n";
				OutputDebugString(output.c_str());
#endif	//_DEBUG
			}
		}

		CloseClipboard();

	}

	void CopySelectionToClipboard() {

		WCHAR * szText = nullptr;
		if( GetListBoxSelectedText(&szText) ) {

			PutStringOnClipboard(szText);
			delete[] szText;

		}

	}
	
	LRESULT CALLBACK llkbhkproc(int code, WPARAM wParam, LPARAM lParam) {

		if( wParam == WM_KEYUP ) {

			auto shCtrl = GetKeyState(VK_CONTROL);
			KBDLLHOOKSTRUCT * kbhk = (KBDLLHOOKSTRUCT *) lParam;

			if( shCtrl >> 15 ) {

				switch( kbhk->vkCode ) {

				case 0x56:	/* V */
				case 0x51:	/* Q */
							
					PostMessage(g_hwnd, WM_CM_INCREMENT, NULL, NULL);

					break;

				case 0x58:	/* X */

					PostMessage(g_hwnd, WM_CLOSE, NULL, NULL);

					break;

				}

			}

		}

		return CallNextHookEx(NULL, code, wParam, lParam);
	}

	void MonitorKeystrokes(bool fMonitor, HOOKPROC proc) {

		static HHOOK hHook = NULL;

		if( fMonitor ) {

			//	Remove any previous hook that was not cleaned up by llhookproc.
			if( hHook != NULL )
				UnhookWindowsHookEx(hHook);

			hHook = SetWindowsHookEx(WH_KEYBOARD_LL, proc, NULL, 0);

		} else {

			UnhookWindowsHookEx(hHook);
			hHook = NULL;

		}

	}	

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

		g_hwnd = hwnd;

		Account * paccount = (Account *) lParam;
		g_pAccount = paccount;
		g_hwndList = GetDlgItem(hwnd, IDC_LIST_CB);
		
		if( paccount->username.size() > 0 )		ListBox_AddString(g_hwndList, paccount->username.c_str());
		if( paccount->password.size() > 0 )		ListBox_AddString(g_hwndList, paccount->password.c_str());

		PutStringOnClipboard(paccount->username.c_str());
		ListBox_SetCurSel(g_hwndList, 0);

		MonitorKeystrokes(true, llkbhkproc);
	
		return TRUE;
	}

	void Cls_OnClose(HWND hwnd) {

		MonitorKeystrokes(false, llkbhkproc);

		PutStringOnClipboard(L"");

		HWND hwndParent = GetParent(hwnd);
		SendMessage(hwndParent, CBM_CLOSING, NULL, NULL);
		
		DestroyWindow(hwnd);
	}

	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

		switch( id ) {
			
		case IDC_LIST_CB :

			if( codeNotify == LBN_SELCHANGE ) {
				CopySelectionToClipboard();
			}

			break;

		}

	}

	INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		int		sel, count;

		switch( message ) {

			HANDLE_DLG_MSG(hwnd, WM_INITDIALOG, Cls_OnInitDialog);		
			HANDLE_DLG_MSG(hwnd, WM_CLOSE, Cls_OnClose);
			HANDLE_DLG_MSG(hwnd, WM_COMMAND, Cls_OnCommand);
			
		case WM_CM_INCREMENT:
						
			sel = ListBox_GetCurSel(g_hwndList);
			count = ListBox_GetCount(g_hwndList);

			++sel;
			if( sel == count )
				sel = 0;

			ListBox_SetCurSel(g_hwndList, sel);

			CopySelectionToClipboard();

			break;

		}

		return FALSE;

	}
}