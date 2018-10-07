#pragma once

#include <string>

#include "ApplicationSettings.h"

class BrowserCommand {

	std::wstring	m_browserpath;
	std::wstring	m_parameters;

	bool			m_fChanged = false;

	void LoadFromRegistry();
	void SaveToRegistry();

	ApplicationSettings * m_pSettings;

public:

	BrowserCommand(ApplicationSettings * pSettings) : m_pSettings{ pSettings } {

		if( !pSettings )	throw std::exception{ "nullptr" };

		LoadFromRegistry();
	}

	const std::wstring& GetBrowserPath() const {
		return m_browserpath;
	}

	const std::wstring& GetParameters() const {
		return m_parameters;
	}

	void SetBrowserPath(std::wstring& path) {
		m_browserpath = path;
		m_fChanged = true;
	}

	void SetParameters(std::wstring& params) {
		m_parameters = params;
		m_fChanged = true;
	}

	~BrowserCommand() {
		if( m_fChanged )
			SaveToRegistry();
	}

};

namespace BrowserCommandDialog {

	INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

}