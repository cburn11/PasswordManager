#include <Windows.h>
#include <windowsx.h>

#include <CommCtrl.h>

#include "resource.h"
#include "pswdgen.h"

#define ComCtl6
#include "CommonHeaders.h"

#pragma comment(lib, "Comctl32.lib")

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
HBRUSH Cls_OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
LRESULT Cls_OnNotify(HWND hwnd, int id, NMHDR * pNMHDR);

HWND	g_hCheckLower, g_hCheckUpper, g_hCheckDigits, g_hCheckSpaces, g_hCheckSymbols;
HWND	g_hEditLower, g_hEditUpper, g_hEditDigits, g_hEditSpaces, g_hEditSymbols;

HINSTANCE g_hInstance;

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch( message ) {

		HANDLE_DLG_MSG(hwnd, WM_INITDIALOG, Cls_OnInitDialog);
		HANDLE_DLG_MSG(hwnd, WM_COMMAND, Cls_OnCommand);
		HANDLE_DLG_MSG(hwnd, WM_CTLCOLORSTATIC, Cls_OnCtlColor);
		HANDLE_DLG_MSG(hwnd, WM_NOTIFY, Cls_OnNotify);

	}

	return FALSE;

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, WCHAR * szCmdLine, int iShowCmd) {
	
	g_hInstance = hInstance;

	INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_UPDOWN_CLASS};

	InitCommonControls();

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DialogProc);

	return 0;
}

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

	//INPUT	keyInputs[2] = {
	//							{INPUT_KEYBOARD, 0},
	//							{INPUT_KEYBOARD, 0}
	//						};
						

	g_hCheckLower = GetDlgItem(hwnd, IDC_CHECK_LOWER);
	g_hCheckUpper = GetDlgItem(hwnd, IDC_CHECK_UPPER);
	g_hCheckDigits = GetDlgItem(hwnd, IDC_CHECK_DIGITS);
	g_hCheckSpaces = GetDlgItem(hwnd, IDC_CHECK_SPACES);
	g_hCheckSymbols = GetDlgItem(hwnd, IDC_CHECK_SYMBOLS);

	g_hEditLower = GetDlgItem(hwnd, IDC_EDIT_LOWER);
	g_hEditUpper = GetDlgItem(hwnd, IDC_EDIT_UPPER);
	g_hEditDigits = GetDlgItem(hwnd, IDC_EDIT_DIGITS);
	g_hEditSpaces = GetDlgItem(hwnd, IDC_EDIT_SPACES);
	g_hEditSymbols = GetDlgItem(hwnd, IDC_EDIT_SYMBOLS);

	Button_SetCheck(g_hCheckLower, TRUE);
	Button_SetCheck(g_hCheckUpper, TRUE);
	Button_SetCheck(g_hCheckDigits, TRUE);

	WCHAR szDef[3] = { 0 };
	int cchDef = sizeof(szDef) / sizeof(szDef[0]);

	int ret = LoadString(g_hInstance, IDS_STRING_LOWER, szDef, cchDef);
	Edit_SetText(g_hEditLower, szDef);

	ret = LoadString(g_hInstance, IDS_STRING_UPPER, szDef, cchDef);
	Edit_SetText(g_hEditUpper, szDef);

	ret = LoadString(g_hInstance, IDS_STRING_DIGITS, szDef, cchDef);
	Edit_SetText(g_hEditDigits, szDef);

	ret = LoadString(g_hInstance, IDS_STRING_SYMBOLS, szDef, cchDef);
	Edit_SetText(g_hEditSymbols, szDef);
	EnableWindow(g_hEditSymbols, FALSE);

	ret = LoadString(g_hInstance, IDS_STRING_SPACES, szDef, cchDef);
	Edit_SetText(g_hEditSpaces, szDef);
	EnableWindow(g_hEditSpaces, FALSE);

	SetWindowLongPtr(g_hCheckLower, GWLP_USERDATA, (LONG_PTR) g_hEditLower);
	SetWindowLongPtr(g_hCheckUpper, GWLP_USERDATA, (LONG_PTR) g_hEditUpper);
	SetWindowLongPtr(g_hCheckDigits, GWLP_USERDATA, (LONG_PTR) g_hEditDigits);
	SetWindowLongPtr(g_hCheckSpaces, GWLP_USERDATA, (LONG_PTR) g_hEditSpaces);
	SetWindowLongPtr(g_hCheckSymbols, GWLP_USERDATA, (LONG_PTR) g_hEditSymbols);

	//	Show keyboard shortcuts automatically
	//keyInputs[0].ki.wVk = VK_MENU;
	//keyInputs[1].ki.wVk = VK_MENU;	keyInputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
	//SendInput(2, keyInputs, sizeof(INPUT));

	return TRUE;
}

void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

	HWND	hwndEdit;
	int		Counts[5] = {0}, i, j, cchPassword;
	WCHAR	* szPassword;


	switch( id ) {

	case IDCANCEL:

		EndDialog(hwnd, 0);
		return;

	case IDC_CHECK_DIGITS:
	case IDC_CHECK_LOWER:
	case IDC_CHECK_SPACES:
	case IDC_CHECK_SYMBOLS:
	case IDC_CHECK_UPPER:

		hwndEdit = (HWND) GetWindowLongPtr(hwndCtl, GWLP_USERDATA);
		if( EnableWindow(hwndEdit, Button_GetCheck(hwndCtl)) )
			SetFocus(hwndEdit);

		return;

	case IDC_BUTTON_GENERATE:

		/*	Prepare count buffer */
		for( i = IDC_CHECK_LOWER, j = 0; i <= IDC_CHECK_SPACES; ++i, ++j ) {

			HWND	hwndCheck = GetDlgItem(hwnd, i);

			if( Button_GetCheck(hwndCheck) ) {

				HWND	hwndEdit = (HWND) GetWindowLongPtr(hwndCheck, GWLP_USERDATA);
				WCHAR	szNum[3] = {0};
				int		count;

				Edit_GetText(hwndEdit, szNum, sizeof(szNum) / sizeof(szNum[0]));
				count = _wtoi(szNum);

				Counts[j] = count;
			}
		}

		if( GeneratePassword((int *) Counts, &szPassword, &cchPassword) ) {
			HWND	hwndEdit;

			hwndEdit = GetDlgItem(hwnd, IDC_EDIT_PASSWORD);

			Edit_SetText(hwndEdit, szPassword);

			if( Button_GetCheck(GetDlgItem(hwnd, IDC_CHECK_COPY)) ) {
				Edit_SetSel(hwndEdit, 0, cchPassword);
				SendMessage(hwndEdit, WM_COPY, 0, 0);
			}

			HeapFree(GetProcessHeap(), 0, szPassword);

		}

		return;

	}

}

HBRUSH Cls_OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type) {

	HWND	hwndEdit;

	hwndEdit = GetDlgItem(hwnd, IDC_EDIT_PASSWORD);

	if(	hwndChild == hwndEdit )
		return (HBRUSH) GetStockObject(WHITE_BRUSH);

	return NULL;
}

LRESULT Cls_OnNotify(HWND hwnd, int id, NMHDR * pNMHDR) {

	NMUPDOWN	* pNMUPDOWN;

	switch( id ) {

	case IDC_SPIN_DIGITS:
	case IDC_SPIN_LOWER:
	case IDC_SPIN_SPACES:
	case IDC_SPIN_SYMBOLS:
	case IDC_SPIN_UPPER:

		pNMUPDOWN = (NMUPDOWN *) pNMHDR;
		pNMUPDOWN->iDelta *= -1;

		break;		

	}

	return 0;
}