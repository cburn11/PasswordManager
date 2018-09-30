#pragma once

#include <Windows.h>

#include <string>

#include "WindowClass.h"
#include "resource.h"

class AccountXMLDialog : public ModalDialog<AccountXMLDialog> {

	std::wstring m_xml;

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

public:

	AccountXMLDialog(const std::wstring& xml);

	INT_PTR MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam);
};