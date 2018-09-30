#pragma once

#include <Windows.h>

#include <WindowClass.h>

#include <vector>
#include <string>

#include "PasswordManager.h"
#include "Accounts.h"
#include "resource.h"

#define FM_FILTER (WM_USER + 1)

class SearchDialog : public ModalDialog<SearchDialog> {

	Accounts *	m_pAccounts;

	HWND		m_hwndCombo;
	HWND		m_hwndEdit;
	HWND		m_hwndParent;

	UINT_PTR	m_idTimer;

	Filter_Type	m_type = Filter_Type::Name;
	std::wstring m_filter = L"";

	const std::vector<const Account *> * m_filteredAccounts = nullptr;

	void ResetComboBox();
	void SetFilterType(Filter_Type type);
	void Filter(const WCHAR * szFilter);

	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

public:

	SearchDialog();

	INT_PTR MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam) override;
};