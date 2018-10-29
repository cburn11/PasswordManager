#include <thread>
#include <memory>

#include "checkhaveibeenpwned_mainwindow.h"
#include "request.h"

#include "resource.h"

CHIBP_MainWindow::CHIBP_MainWindow() {

	m_hInstance = (HINSTANCE) GetModuleHandle(nullptr);

	m_szTemplate = MAKEINTRESOURCE(IDD_DIALOG_MAINWINDOW);

	m_pAppSettings = std::make_unique<ApplicationSettings>(std::wstring{ L"SOFTWARE\\Clifford\\checkhaveibeenpwned" });
}

CHIBP_MainWindow::~CHIBP_MainWindow() {

}

INT_PTR CHIBP_MainWindow::MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam) {
		
	switch( message ) {

		HANDLE_DLG_MSG_RET(m_window, WM_COMMAND, Cls_OnCommand, __super::MessageHandler(message, wParam, lParam));
		
		HANDLE_DLG_MSG_DERIVED(m_window, WM_INITDIALOG, Cls_OnInitDialog, __super::MessageHandler(message, wParam, lParam));

		HANDLE_DLG_MSG(m_window, WM_SIZE, Cls_OnSize);
		HANDLE_DLG_MSG(m_window, WM_GETMINMAXINFO, Cls_OnGetMinMaxInfo);

	case CHIBP_ADDRESULT:
		AddResult( std::unique_ptr<std::wstring>{ reinterpret_cast<std::wstring *>( lParam ) } );
		break;
	}

	return __super::MessageHandler(message, wParam, lParam);
}

void CHIBP_MainWindow::Cls_OnSize(HWND hwnd, UINT state, int cx, int cy) {

	int cxCheck = m_rectCheck.right - m_rectCheck.left;
	int cyCheck = m_rectCheck.bottom - m_rectCheck.top;
	
	int cxAccount = cx - 3 * pad - cxCheck;
	int cyAccount = m_rectAccount.bottom - m_rectAccount.top;
	
	int cxResults = cx - 2 * pad;
	int cyResults = cy - 3 * pad - cyAccount;
	
	MoveWindow(m_hwndCheck, 2 * pad + cxAccount, pad, cxCheck, cyCheck, TRUE);
	MoveWindow(m_hwndAccount, pad, pad, cxAccount, cyAccount, TRUE);
	MoveWindow(m_hwndResults, pad, 2 * pad + cyAccount, cxResults, cyResults, TRUE);

	RECT rectCurrent;
	GetWindowRect(m_window, &rectCurrent);
	int x = rectCurrent.left;
	int y = rectCurrent.top;
	int width = rectCurrent.right - x;
	int height = rectCurrent.bottom - y;

	if( width > 0 && height > 0 ) {

		m_pAppSettings->width = width;
		m_pAppSettings->height = height;
	}

	if( x > 0 && y > 0 ) {

		m_pAppSettings->x = x;
		m_pAppSettings->y = y;
	}
}

void CHIBP_MainWindow::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

	switch( id ) {

	case IDC_BUTTON_CHECK:
		CheckAccount();
		break;
	}

}

BOOL CHIBP_MainWindow::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

	m_hwndCheck = GetDlgItem(m_window, IDC_BUTTON_CHECK);
	m_hwndAccount = GetDlgItem(m_window, IDC_EDIT_ACCOUNT);
	m_hwndResults = GetDlgItem(m_window, IDC_EDIT_RESULTS);

	GetWindowRect(m_hwndCheck, &m_rectCheck);
	GetWindowRect(m_hwndAccount, &m_rectAccount);
	GetWindowRect(m_hwndResults, &m_rectResults);

	GetWindowRect(m_window, &m_rectMin);

	SetWindowPos(m_window, NULL, m_pAppSettings->x, m_pAppSettings->y,
		m_pAppSettings->width, m_pAppSettings->height, SWP_NOZORDER);

	SetFocus(m_hwndAccount);

	return FALSE;
}

void CHIBP_MainWindow::Cls_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo) {

	lpMinMaxInfo->ptMinTrackSize.x = m_rectMin.right - m_rectMin.left;
	lpMinMaxInfo->ptMinTrackSize.y = m_rectMin.bottom - m_rectMin.top;
}

void CHIBP_MainWindow::SetAccount(const std::wstring& account) {

	Edit_SetText(m_hwndAccount, account.c_str());
}

void CHIBP_MainWindow::CheckAccount() {

	auto strAccount = GetAccount();
	if( strAccount.length() < 1 )
		return;
	
	Edit_SetSel(m_hwndAccount, 0, strAccount.length());

	HWND hwnd = m_window;

	std::thread check_thread{ [hwnd](std::wstring&& strAccount) {

		auto json = ::CheckAccount(strAccount);

		// Pointer freed by main window
		auto p_strResult = new std::wstring{ L"Search for " + strAccount + L" returned " };

		if( p_strResult ) {
			*p_strResult += ParseJSONResponse(json);
			auto res = PostMessage(hwnd, CHIBP_ADDRESULT, 0, reinterpret_cast<LPARAM>( p_strResult ));
#ifdef _DEBUG
			auto error = GetLastError();
			std::wstring out{ L"check_thread, PostMessage returned false, error = " };
			out += std::to_wstring(error);
			OutputDebugString(out.c_str());
#endif //_DEBUG
		}

	}, std::move(strAccount) };

	check_thread.detach();
}

std::wstring CHIBP_MainWindow::GetAccount() {

	auto cch = Edit_GetTextLength(m_hwndAccount);
	if( cch < 1 )
		return L"";

	std::unique_ptr<WCHAR> pszAccount{ new WCHAR[cch + 1]{0} };

	Edit_GetText(m_hwndAccount, pszAccount.get(), cch + 1);

	return pszAccount.get();
}

void CHIBP_MainWindow::AddResult(std::unique_ptr<std::wstring> p_strResult) {

	if( !p_strResult.get() )		return;

	if( p_strResult->length() < 1 )	return;

	if( m_searchResults.length() > 0 )
		m_searchResults += L"\r\n\r\n----------------\r\n\r\n";

	m_searchResults += *p_strResult;

	UpdateResults();
}

void CHIBP_MainWindow::UpdateResults() {

	Edit_SetText(m_hwndResults, m_searchResults.c_str());
}