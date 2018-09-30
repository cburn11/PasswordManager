#pragma once

#include <Windows.h>
#include <windowsx.h>

#include <CommonHeaders.h>
#include <WindowClass.h>

#include <memory>
#include <string>

#include "ApplicationSettings.h"

#define CHIBP_ADDRESULT (WM_USER + 15)

class CHIBP_MainWindow : public ApplicationModelessDialog<CHIBP_MainWindow> {
	
public:

	CHIBP_MainWindow();
	~CHIBP_MainWindow();

	INT_PTR MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam) override;

	void SetAccount(const std::wstring& account);
	void CheckAccount();

private:

	const int pad = 5;

	HWND m_hwndCheck;
	HWND m_hwndAccount;
	HWND m_hwndResults;

	RECT m_rectCheck;
	RECT m_rectAccount;
	RECT m_rectResults;
	RECT m_rectMin;

	std::wstring m_searchResults;

	std::unique_ptr<ApplicationSettings> m_pAppSettings;
	
	std::wstring GetAccount();

	void AddResult(std::unique_ptr<std::wstring> p_strResult);
	void UpdateResults();

	void Cls_OnSize(HWND hwnd, UINT state, int cx, int cy);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

};