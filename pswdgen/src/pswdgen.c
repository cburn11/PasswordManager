#define COBJMACROS

#include <Windows.h>
#include <windowsx.h>

#include <CommCtrl.h>

#include <string.h>

#include "resource.h"

#include "pswdgen_h.h"
#include "pswdgen_Application.h"

#define ComCtl6
#include "CommonHeaders.h"

#pragma comment(lib, "Comctl32.lib")

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
HBRUSH Cls_OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
LRESULT Cls_OnNotify(HWND hwnd, int id, NMHDR * pNMHDR);
void Cls_OnDestroy(HWND hwnd);

HWND	g_hCheckLower, g_hCheckUpper, g_hCheckDigits, g_hCheckSpaces, g_hCheckSymbols;
HWND	g_hEditLower, g_hEditUpper, g_hEditDigits, g_hEditSpaces, g_hEditSymbols;

HWND	g_hwndMain;

BSTR	g_bstr_cLower, g_bstr_cUpper, g_bstr_cDigits, g_bstr_cSpaces, g_bstr_cSymbols;
BSTR	g_bstr_lower, g_bstr_upper, g_bstr_digits, g_bstr_spaces, g_bstr_symbols;

IPasswordGenerator* g_p_pswdgen = NULL;

HINSTANCE g_hInstance;

BOOL	g_fEmbedding = FALSE;
BOOL	g_fSettingsMode = FALSE;

void SetMode(BSTR mode) {

	HWND hwndEdit = GetDlgItem(g_hwndMain, IDC_EDIT_PASSWORD);
	HWND hwndCheck = GetDlgItem(g_hwndMain, IDC_CHECK_COPY);
	HWND hwndButton = GetDlgItem(g_hwndMain, IDC_BUTTON_GENERATE);

	if( wcscmp(mode, L"Settings") == 0 ) {
		
		EnableWindow(hwndEdit, FALSE);
		SetWindowText(hwndEdit, L"Settings mode ...");

		EnableWindow(hwndCheck, FALSE);
		EnableWindow(hwndButton, FALSE);

		g_fSettingsMode = TRUE;

	} else if( wcscmp(mode, L"Regular") == 0 ) {

		EnableWindow(hwndEdit, TRUE);
		SetWindowText(hwndEdit, L"");

		EnableWindow(hwndCheck, TRUE);
		EnableWindow(hwndButton, TRUE);

		g_fSettingsMode = FALSE;
	}

	void* p_v_app = GetWindowLongPtr(g_hwndMain, GWLP_USERDATA);
	BSTR name = SysAllocString(L"Mode");
	if( !name )	return;
	TriggerPropertyChange(p_v_app, name, mode);
	SysFreeString(name);
}

void ReadEditTextIntoBSTR(HWND hwndEdit, BSTR* pbstr) {

	if( !pbstr )
		return;

	if( *pbstr )
		SysFreeString(*pbstr);

	if( IsWindowEnabled(hwndEdit) ) {

		int cch = Edit_GetTextLength(hwndEdit);
		*pbstr = SysAllocStringLen(NULL, cch);
		Edit_GetText(hwndEdit, *pbstr, cch + 1);

	} else {

		*pbstr = SysAllocString(L"0");

	}
}

BSTR GeneratePassword() {

	HRESULT hr;
	BSTR password = NULL;

	ReadEditTextIntoBSTR(g_hEditLower, &g_bstr_cLower);
	ReadEditTextIntoBSTR(g_hEditUpper, &g_bstr_cUpper);
	ReadEditTextIntoBSTR(g_hEditDigits, &g_bstr_cDigits);
	ReadEditTextIntoBSTR(g_hEditSymbols, &g_bstr_cSymbols);
	ReadEditTextIntoBSTR(g_hEditSpaces, &g_bstr_cSpaces);

	hr = IPasswordGenerator_SetProperty(g_p_pswdgen, g_bstr_lower, g_bstr_cLower, NULL);
	hr = IPasswordGenerator_SetProperty(g_p_pswdgen, g_bstr_upper, g_bstr_cUpper, NULL);
	hr = IPasswordGenerator_SetProperty(g_p_pswdgen, g_bstr_digits, g_bstr_cDigits, NULL);
	hr = IPasswordGenerator_SetProperty(g_p_pswdgen, g_bstr_symbols, g_bstr_cSymbols, NULL);
	hr = IPasswordGenerator_SetProperty(g_p_pswdgen, g_bstr_spaces, g_bstr_cSpaces, NULL);

	hr = IPasswordGenerator_GeneratePassword(g_p_pswdgen, &password);
	if( S_OK == hr && ! g_fSettingsMode ) {
		Edit_SetText(GetDlgItem(g_hwndMain, IDC_EDIT_PASSWORD), password);
	}

	return password;
}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch( message ) {

		HANDLE_DLG_MSG(hwnd, WM_INITDIALOG, Cls_OnInitDialog);
		HANDLE_DLG_MSG(hwnd, WM_COMMAND, Cls_OnCommand);
		HANDLE_DLG_MSG(hwnd, WM_CTLCOLORSTATIC, Cls_OnCtlColor);
		HANDLE_DLG_MSG(hwnd, WM_NOTIFY, Cls_OnNotify);
		HANDLE_DLG_MSG(hwnd, WM_DESTROY, Cls_OnDestroy);

	case WM_APP_GENERATE_PASSWORD: {
		BSTR* pbstr = ( BSTR* ) lParam;
		*pbstr = GeneratePassword();
		return SetDlgMsgResult(hwnd, message, 0);
	}

	case WM_APP_GET_PROPERTY: {

		BSTR property_name = ( BSTR ) wParam;
		BSTR* p_property_value = ( BSTR* ) lParam;

		HRESULT hr = IPasswordGenerator_GetProperty(g_p_pswdgen, property_name, p_property_value);

		return SetDlgMsgResult(hwnd, message, hr == S_OK ? 0 : 1);
	}


	case WM_APP_SET_PROPERTY: {

		BSTR property_name = ( BSTR ) wParam;
		BSTR property_value = ( BSTR ) lParam;

		if( wcscmp(property_name, L"Mode") == 0 ) {
			LRESULT res = 0;
			SetMode(property_value);
			return SetDlgMsgResult(hwnd, message, res);
		}

		HRESULT hr;
		VARIANT_BOOL ret;
		hr = IPasswordGenerator_SetProperty(g_p_pswdgen, property_name, property_value, &ret);

		if( S_OK == hr ) {

			if( wcscmp(property_name, L"c_lower_case") == 0 )
				Edit_SetText(g_hEditLower, property_value);
			else if( wcscmp(property_name, L"c_upper_case") == 0 )
				Edit_SetText(g_hEditUpper, property_value);
			else if( wcscmp(property_name, L"c_digits") == 0 )
				Edit_SetText(g_hEditDigits, property_value);
			else if( wcscmp(property_name, L"c_symbols") == 0 )
				Edit_SetText(g_hEditSymbols, property_value);
			else if( wcscmp(property_name, L"w_spaces") == 0 )
				Edit_SetText(g_hEditSpaces, property_value);

			return SetDlgMsgResult(hwnd, message, ret == VARIANT_TRUE ? 0 : 1);
		}

	}

	}

	return FALSE;

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, WCHAR * szCmdLine, int iShowCmd) {
	
	g_hInstance = hInstance;

	HRESULT hr = OleInitialize(NULL);

	hr = CoCreateInstance(&CLSID_PasswordGenerator, NULL, CLSCTX_INPROC_SERVER,
		&IID_IPasswordGenerator, &g_p_pswdgen);
	if( S_OK != hr )	return 1;
	
	INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_UPDOWN_CLASS};

	InitCommonControls();

	void* papp = NULL;

	HWND hwnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DialogProc);
	g_hwndMain = hwnd;

	if( wcscmp(szCmdLine, L"-Embedding") == 0 ) {

		g_fEmbedding = TRUE;
		void* papp = CreateApplication(g_p_pswdgen);

	} else {

		UpdateWindow(g_hwndMain);
		ShowWindow(g_hwndMain, iShowCmd);
	}

	MSG msg;
	while( GetMessage(&msg, NULL, 0, 0) ) {
		
		if( !IsDialogMessage(g_hwndMain, &msg) ) {

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		
		}

	}

	IPasswordGenerator_Release(g_p_pswdgen);

	if( papp )	DeleteApplication(papp);

	return 0;
}

BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {				

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

	g_bstr_lower	= SysAllocString(L"c_lower_case");
	g_bstr_upper	= SysAllocString(L"c_upper_case");
	g_bstr_digits	= SysAllocString(L"c_digits");
	g_bstr_symbols	= SysAllocString(L"c_symbols");
	g_bstr_spaces	= SysAllocString(L"c_spaces");

	g_bstr_cLower = NULL;
	g_bstr_cUpper = NULL;
	g_bstr_cDigits = NULL;
	g_bstr_cSymbols = NULL;
	g_bstr_cSpaces = NULL;

	return TRUE;
}

void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

	HWND	hwndEdit;
	int		Counts[5] = {0}, i, j, cchPassword;
	WCHAR	* szPassword;


	switch( id ) {

	case IDCANCEL:
		if( g_fEmbedding == TRUE )
			ShowWindow(hwnd, SW_HIDE);
		else
			DestroyWindow(hwnd);

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

	case IDC_BUTTON_GENERATE: {
		
		BSTR password = GeneratePassword();
		void* p_v_app = GetWindowLongPtr(hwnd, GWLP_USERDATA);
		TriggerPasswordGenerated(p_v_app, password);
		SysFreeString(password);

		return; }

	case IDC_EDIT_LOWER:
	case IDC_EDIT_UPPER:
	case IDC_EDIT_DIGITS:
	case IDC_EDIT_SYMBOLS:
	case IDC_EDIT_SPACES:

		if( codeNotify == EN_CHANGE ) {
			
			BSTR value = NULL;
			ReadEditTextIntoBSTR(hwndCtl, &value);
			void* p_v_app = GetWindowLongPtr(hwnd, GWLP_USERDATA);

			BSTR name = NULL;
			switch( id ) {
			case IDC_EDIT_LOWER:
				name = SysAllocString(L"c_lower_case");
				break;
			case IDC_EDIT_UPPER:
				name = SysAllocString(L"c_upper_case");
				break;
			case IDC_EDIT_DIGITS:
				name = SysAllocString(L"c_digits");
				break;
			case IDC_EDIT_SYMBOLS:
				name = SysAllocString(L"c_symbols");
				break;
			case IDC_EDIT_SPACES:
				name = SysAllocString(L"c_spaces");
			}

			TriggerPropertyChange(p_v_app, name, value);

			SysFreeString(name);
			SysFreeString(value);
		}

		break;
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

void Cls_OnDestroy(HWND hwnd) {

	void* p_v_app = GetWindowLongPtr(hwnd, GWLP_USERDATA);

	TriggerQuit(p_v_app);

	PostQuitMessage(0);
}