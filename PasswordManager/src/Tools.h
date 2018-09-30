#pragma once

#include <Windows.h>

#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include "WindowClass.h"
#include "resource.h"

using tool_pair = std::pair<std::wstring, std::wstring>;

class Tools : public ModalDialog<Tools> {	

	std::vector<tool_pair> m_tools;
	std::vector<tool_pair> m_toolsDelta;
	
	bool m_fToolsChanged = false;

	bool LoadToolsFromRegistry();
	bool SaveToolsToRegistry();

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnNotify(HWND hwnd, int id, NMHDR * pNMHDR);

	void UpdateControls();
	void PopulateComboBox();
	void AddButtonClick();
	void BrowseButtonClick();
	void RemoveButtonClick();

	void AddTool(std::wstring caption, std::wstring target);
	void RemoveTool(int index);

	tool_pair& GetToolPair(int index);

public:

	Tools();

	~Tools();

	INT_PTR MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam);

	std::vector<std::wstring> GetCaptions();

	const std::wstring& GetTarget(int index);

};