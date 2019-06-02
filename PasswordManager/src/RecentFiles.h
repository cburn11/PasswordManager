#pragma once

#define RECENT_FILE_MENU_ID 50000
#define RECENT_FILE_MENU_CLEAR 50010

#include <vector>
#include <string>

#include "ApplicationSettings.h"

class RecentFiles {

	std::vector<std::wstring> m_vFilepaths;
	bool m_fChanged = false;
	ApplicationSettings * m_pSettings;
	HMENU m_hRecentFiles = NULL;

	void LoadRecentFilesFromRegistry();
	void SaveRecentFilesInRegistry();

	bool IsFilepathAlreadyInList(const wchar_t * szFilepath);

	HMENU CreateRecentFilesMenu();
	void DestroyRecentFilesMenu();
	void UpdateRecentFilesMenu();

	void ClearMenu(HMENU hMenu);

public:

	RecentFiles(ApplicationSettings * pSettings) : m_pSettings{ pSettings } {

		if( !pSettings )	throw std::exception{ "nullptr" };

		m_vFilepaths.reserve(5);

		LoadRecentFilesFromRegistry();

		m_hRecentFiles = CreateRecentFilesMenu();
		UpdateRecentFilesMenu();
	}

	~RecentFiles() {
		if( m_fChanged )
			SaveRecentFilesInRegistry();

		DestroyRecentFilesMenu();
	}

	void FileOpened(const wchar_t * szFilepath);

	const std::vector<std::wstring>& GetRecentFiles() { return m_vFilepaths; }

	void RemoveRecentFile(UINT index);

	void ClearRecentFiles() { m_vFilepaths.clear(); m_fChanged = true; UpdateRecentFilesMenu(); }

	HMENU GetRecentFilesMenu() { return m_hRecentFiles; }
};