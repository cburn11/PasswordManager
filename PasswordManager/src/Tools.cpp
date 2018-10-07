#include <Windows.h>
#include <windowsx.h>

#include "PasswordManager.h"

#include "Tools.h"
#include "CommonHeaders.h"
#include "resource.h"

using std::vector;
using std::wstring;
using std::pair;
using std::begin;
using std::end;

HWND g_hwndCombo;
HWND g_hwndCaption;
HWND g_hwndTarget;
HWND g_hwndAdd;
HWND g_hwndBrowse;
HWND g_hwndRemove;

bool GetFilePath(HWND hwnd, WCHAR ** pszPath) {

	WCHAR			* szFilename = nullptr;
	OPENFILENAME	ofn = { sizeof(ofn), 0 };

	szFilename = new WCHAR[MAX_PATH]{ 0 };
	if( !szFilename ) {
		*pszPath = nullptr;
		return false;
	}

	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = sizeof(WCHAR) * MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	DWORD error;
	BOOL ret;

	ret = GetOpenFileName(&ofn);
	if( !ret ) {
		*pszPath = nullptr;
		delete szFilename;
		return false;
	}

	*pszPath = szFilename;

	return true;
}

WCHAR * GetComboText() {

	int sel = ComboBox_GetCurSel(g_hwndCombo);
	int cch = ComboBox_GetLBTextLen(g_hwndCombo, sel);
	
	WCHAR * szComboEntry = new WCHAR[cch + 1];
	
	if( szComboEntry ) {
		
		ComboBox_GetLBText(g_hwndCombo, sel, szComboEntry);
	}

	return szComboEntry;
}

WCHAR * GetEditText(HWND hwndEdit) {
	
	int cch = Edit_GetTextLength(hwndEdit);

	WCHAR * szEditText = new WCHAR[cch + 1];

	if( szEditText ) {

		Edit_GetText(hwndEdit, szEditText, cch + 1);
	}

	return szEditText;
}

Tools::Tools(ApplicationSettings * pSettings) : m_pSettings{ pSettings } {

	if( !pSettings )	throw std::exception{ "nullptr" };

	m_hInstance = GetModuleHandle(NULL);
	m_szTemplate = MAKEINTRESOURCE(IDD_DIALOG_TOOLS);

	m_tools.reserve(5);

	LoadToolsFromRegistry();
}

Tools::~Tools() {

	if( m_fToolsChanged )	SaveToolsToRegistry();

}

bool Tools::LoadToolsFromRegistry() {

	const WCHAR * szSubStrings = m_pSettings->getMultiSZ(L"Tools");

	if( !szSubStrings )		return false;

	for( auto cch = wcslen(szSubStrings); cch > 0; cch = wcslen(szSubStrings) ) {

		wstring caption{ szSubStrings };
		szSubStrings += cch + 1;

		cch = wcslen(szSubStrings);
		wstring target{ szSubStrings };
		szSubStrings += cch + 1;

		m_tools.push_back(tool_pair{ caption, target });

	}
		
	return true;
}

bool Tools::SaveToolsToRegistry() {
	
	ULONG size = 0;
	for_each(begin(m_tools), end(m_tools),
		[&size](tool_pair const & p) {
		size += p.first.length() + 1;		// cch + null
		size += p.second.length() + 1;		// cch + null
	});
	
	auto dsz = SysAllocStringLen(nullptr, size);	//	SysAllocStringLen allocates size + 1 for the terminating null
													//	But a subsequent call to SysStringLen or SysStringByteLen will
													//	return size, not the allocated size to terminating null
	//Don't call SysFreeString(dsz), dsz is now owned by the internal structure of ApplicationSettings		

	if( dsz ) {
		memset(dsz, 0, size * sizeof(dsz[0]));
		size = 0;
		for_each(begin(m_tools), end(m_tools),
			[&size, dsz](tool_pair const & p) {
			size += wsprintf(dsz + size, L"%s", p.first.c_str());
			++size;
			size += wsprintf(dsz + size, L"%s", p.second.c_str());
			++size;
		});

		m_pSettings->setMultiSZ(L"Tools", dsz);
		m_fToolsChanged = false;
	}
			
	return true;
}

BOOL Tools::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
	
	m_toolsDelta = m_tools;

	g_hwndCombo		= GetDlgItem(hwnd, IDC_COMBO_TOOLS);
	
	g_hwndCaption	= GetDlgItem(m_window, IDC_EDIT_TOOL_CAPTION);
	g_hwndTarget	= GetDlgItem(m_window, IDC_EDIT_TOOL_TARGET);
	g_hwndAdd		= GetDlgItem(m_window, IDC_BUTTON_TOOL_ADD);
	g_hwndRemove	= GetDlgItem(m_window, IDC_BUTTON_TOOL_REMOVE);
	g_hwndBrowse	= GetDlgItem(m_window, IDC_BUTTON_TOOL_BROWSE);

	PopulateComboBox();

	ComboBox_SetCurSel(g_hwndCombo, ComboBox_GetCount(g_hwndCombo) - 1);
	SetFocus(g_hwndCombo);

	UpdateControls();

	return FALSE;
}

void Tools::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {

	switch( id ) {

	case IDC_COMBO_TOOLS:

#ifdef DEBUG
	{
		wstring out{ L"Notification: " };
		out += std::to_wstring(codeNotify);
		out += L"\n";
		OutputDebugString(out.c_str());
	}
#endif // DEBUG


		if( codeNotify == CBN_SELCHANGE ) {

			UpdateControls();
		}

		break;

	case IDC_BUTTON_TOOL_ADD:

		AddButtonClick();

		break;

	case IDC_BUTTON_TOOL_BROWSE:

		BrowseButtonClick();

		break;

	case IDC_BUTTON_TOOL_REMOVE:

		RemoveButtonClick();

		break;

	case IDOK:

		if( m_fToolsChanged )
			m_tools = m_toolsDelta;

		EndDialog(m_window, 0);

		break;
	}
}

void Tools::Cls_OnNotify(HWND hwnd, int id, NMHDR * pNMHDR) {

}

INT_PTR Tools::MessageHandler(UINT const message, WPARAM const wParam, LPARAM const lParam) {

	auto ret = __super::MessageHandler(message, wParam, lParam);

	switch( message) {

		HANDLE_DLG_MSG(m_window, WM_COMMAND, Cls_OnCommand);
		HANDLE_DLG_MSG(m_window, WM_INITDIALOG, Cls_OnInitDialog);
		HANDLE_DLG_MSG(m_window, WM_NOTIFY, Cls_OnNotify);

	}
	
	return ret;
}

tool_pair& Tools::GetToolPair(int index) {

	return m_toolsDelta.at(index);

}

void Tools::UpdateControls() {

	SetFocus(g_hwndCaption);
	
	auto szComboEntry = GetComboText();
	if( !szComboEntry ) return;

	if( wcscmp(L"[New Entry]", szComboEntry) == 0 ) {

		Button_SetText(g_hwndAdd, L"Add");
		Button_Enable(g_hwndRemove, FALSE);

		Edit_SetText(g_hwndCaption, L"");
		Edit_SetText(g_hwndTarget, L"");


	} else {

		int sel = ComboBox_GetCurSel(g_hwndCombo);
		if( sel >= m_toolsDelta.size() )	return;

		auto& tp = GetToolPair(sel);

		Edit_SetText(g_hwndCaption, tp.first.c_str());
		Edit_SetText(g_hwndTarget, tp.second.c_str());

		Button_SetText(g_hwndAdd, L"Update");
		Button_Enable(g_hwndRemove, TRUE);
	}

	delete[] szComboEntry;
}

void Tools::PopulateComboBox() {

	ComboBox_ResetContent(g_hwndCombo);

	for( const auto& tp : m_toolsDelta ) {
		ComboBox_AddString(g_hwndCombo, tp.first.c_str());
	}

	ComboBox_AddString(g_hwndCombo, L"[New Entry]");
}

void Tools::AddButtonClick() {

	auto szComboEntry = GetComboText();
	if( !szComboEntry ) return;

	WCHAR * szCaption = GetEditText(g_hwndCaption);
	if( !szCaption )	return;

	WCHAR * szTarget = GetEditText(g_hwndTarget);
	if( !szTarget )		return;
	

	if( wcscmp(L"[New Entry]", szComboEntry) == 0 ) {

		AddTool(szCaption, szTarget);

		PopulateComboBox(); 

		ComboBox_SetCurSel(g_hwndCombo, ComboBox_GetCount(g_hwndCombo) - 2);

		UpdateControls();

	} else {

		int sel = ComboBox_GetCurSel(g_hwndCombo);

		auto& tp = GetToolPair(sel);

		tp.first = szCaption;

		tp.second = szTarget;

		m_fToolsChanged = true;

	}

	delete[] szCaption;
	delete[] szTarget;
	delete[] szComboEntry;
}

void Tools::AddTool(std::wstring caption, std::wstring target) {

	m_toolsDelta.push_back(tool_pair{ std::move(caption), std::move(target) });

	m_fToolsChanged = true;

}

void Tools::RemoveTool(int index) {

	if( index >= m_toolsDelta.size() )	return;

	auto it = std::begin(m_toolsDelta) + index;
	m_toolsDelta.erase(it);
	m_fToolsChanged = true;
}

void Tools::BrowseButtonClick() {

	WCHAR * szTarget = nullptr;
	if( GetFilePath(m_window, &szTarget) ) {

		Edit_SetText(g_hwndTarget, szTarget);

		delete[] szTarget;
	}

}

void Tools::RemoveButtonClick() {

	int sel = ComboBox_GetCurSel(g_hwndCombo);

	RemoveTool(sel);

	PopulateComboBox();

	ComboBox_SetCurSel(g_hwndCombo, sel - 1);

	UpdateControls();

}

vector<wstring> Tools::GetCaptions() {

	vector<wstring> captions;

	captions.reserve(m_tools.size());

	for( const auto& tp : m_tools ) {
		captions.push_back(tp.first);
	}

	return captions;
}

const std::wstring& Tools::GetTarget(int index) {

	const tool_pair& tp = m_tools.at(index);

	return tp.second;

}