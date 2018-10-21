#include "SearchDialog.h"

#include <memory>
using std::unique_ptr;

#include <string>
using std::wstring;

#include <windowsx.h>
#include <CommonHeaders.h>

namespace SearchDialogFunctions {

	LRESULT CALLBACK ComboWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		WNDPROC oldproc = (WNDPROC) GetWindowLongPtr(hwnd, GWLP_USERDATA);

		switch( message ) {

		case CB_SHOWDROPDOWN: {
			
			auto ret = CallWindowProc(oldproc, hwnd, message, wParam, lParam);
			
			if( ComboBox_GetCount(hwnd) > 1 )
				ComboBox_SetCurSel(hwnd, -1);
				
			return ret;
		}

		}

		return CallWindowProc(oldproc, hwnd, message, wParam, lParam);
	}

	LRESULT CALLBACK EditWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

		WNDPROC oldproc = (WNDPROC) GetWindowLongPtr(hwnd, GWLP_USERDATA);
		auto ret = CallWindowProc(oldproc, hwnd, message, wParam, lParam);

		switch( message ) {
		case WM_KEYDOWN: {

			if( VK_RIGHT == wParam ) {
				ComboBox_ShowDropdown(GetParent(hwnd), TRUE);
			}

			break; }
		}

		return ret;
	}

	void CALLBACK TimerProc(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime) {

		KillTimer(hwnd, idEvent);

		HWND hwndCombo = GetDlgItem(hwnd, IDC_COMBO_ACCOUNTS);

		int cchFilter = ComboBox_GetTextLength(hwndCombo);
		WCHAR * szFilter = new WCHAR[cchFilter + 1]{ 0 };

		if( szFilter ) {

			ComboBox_GetText(hwndCombo, szFilter, cchFilter + 1);
			SendMessage(hwnd, FM_FILTER, 0, (LPARAM) szFilter);
			delete[] szFilter;
		}
	}
}

SearchDialog::SearchDialog() {

	m_hInstance = GetModuleHandle(NULL);
	m_szTemplate = MAKEINTRESOURCE(IDD_DIALOG_SEARCH);
}

INT_PTR SearchDialog::MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam) {

	switch( message ) {

		HANDLE_DLG_MSG(m_window, WM_INITDIALOG, Cls_OnInitDialog);
		HANDLE_DLG_MSG(m_window, WM_COMMAND, Cls_OnCommand);

	case FM_FILTER:

		Filter((const WCHAR *) lParam);

		break;
	}

	auto ret = __super::MessageHandler(message, wParam, lParam);
	return ret;
}

BOOL SearchDialog::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {

	m_hwndCombo = GetDlgItem(hwnd, IDC_COMBO_ACCOUNTS);
	auto oldproc = SetWindowLongPtr(m_hwndCombo, GWLP_WNDPROC, (LONG_PTR) SearchDialogFunctions::ComboWndProc);
	SetWindowLongPtr(m_hwndCombo, GWLP_USERDATA, oldproc);
	
	m_hwndEdit = GetFirstChild(m_hwndCombo);
	oldproc = SetWindowLongPtr(m_hwndEdit, GWLP_WNDPROC, (LONG_PTR) SearchDialogFunctions::EditWndProc);
	SetWindowLongPtr(m_hwndEdit, GWLP_USERDATA, oldproc);

	m_hwndParent = GetParent(hwnd);
	UserData * pUserData = (UserData *) GetWindowLongPtr(m_hwndParent, GWLP_USERDATA);
	
	m_pAccounts = pUserData->pAccounts;

	ResetComboBox();

	Button_SetCheck(GetDlgItem(hwnd, IDC_RADIO_NAME), TRUE);

	SetFocus(m_hwndCombo);

	return FALSE;
}

void SearchDialog::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

	INT_PTR ret = -1;

	switch( id ) {

	case IDC_RADIO_NAME:
	case IDC_RADIO_URL:
	case IDC_RADIO_USERNAME:
	case IDC_RADIO_DESCRIPTION:

		SetFilterType((Filter_Type) id);

		break;

	case IDOK: {

		auto cur_sel = ComboBox_GetCurSel(m_hwndCombo);

		if( m_filteredAccounts ) {

			if( cur_sel > -1 && cur_sel < m_filteredAccounts->size() ) {
				auto paccount = m_filteredAccounts->at(cur_sel);
				ret = (INT_PTR) paccount;
			}

		} else {

			auto& accounts = m_pAccounts->GetAccounts();

			if( cur_sel > -1 && cur_sel < accounts.size() ) {
				auto& account = accounts.at(cur_sel);
				ret = (INT_PTR) &account;
			}

		}
	}
		//	Fall through to IDCANCEL

	case IDCANCEL:

		EndDialog(hwnd, ret);
		break;

	case IDC_COMBO_ACCOUNTS:

		if( CBN_EDITCHANGE == codeNotify ) {

			KillTimer(hwnd, m_idTimer);
			m_idTimer = SetTimer(hwnd, 0, 500, SearchDialogFunctions::TimerProc);
		
		} 

		break;
	}
}

void SearchDialog::ResetComboBox() {

	ComboBox_ResetContent(m_hwndCombo);
	
	auto& accounts = m_pAccounts->GetAccounts();
	for( auto& account : accounts ) {
		ComboBox_AddString(m_hwndCombo, account.to_string().c_str());
	}

	m_filteredAccounts = nullptr;
}

void SearchDialog::Filter(const WCHAR * szFilter) {

	if( !szFilter )	return;

	m_filter = szFilter;

	if( wcslen(szFilter) == 0 ) {
		ResetComboBox();
		return;
	}

	m_filteredAccounts = m_pAccounts->Filter(szFilter, m_type);

	if( m_filteredAccounts ) {

		ComboBox_ResetContent(m_hwndCombo);

		for( auto paccount : *m_filteredAccounts ) {

			wstring str = paccount->to_string();

			if( m_type != Filter_Type::Name && m_type != Filter_Type::Username ) {

				str += L" (";

				switch( m_type ) {

				case Filter_Type::Url:
					str += L"URL: ";
					str += paccount->url;
					break;

				case Filter_Type::Description:
					str += L"Description: ";
					str += paccount->description;
					break;
				}

				str += L")";
			}

			ComboBox_AddString(m_hwndCombo, str.c_str());
		}

		ComboBox_ShowDropdown(m_hwndCombo, TRUE);

		// Preserve search string after showing drop down
		ComboBox_SetText(m_hwndCombo, m_filter.c_str());
		ComboBox_SetEditSel(m_hwndCombo, m_filter.length(), m_filter.length());
		
	}
}

void SearchDialog::SetFilterType(Filter_Type type) {

	if( m_type != type ) {
		m_type = type;

		if( m_filter.size() > 0 )
			Filter(m_filter.c_str());
	}
}