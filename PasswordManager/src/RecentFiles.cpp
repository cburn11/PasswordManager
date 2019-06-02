#include <Windows.h>

#include <stdlib.h>

#include <vector>
#include <string>
#include <algorithm>

using std::vector;
using std::wstring;
using std::for_each;
using std::find_if;
using std::begin;
using std::end;

#include "PasswordManager.h"
#include "RecentFiles.h"

void RecentFiles::LoadRecentFilesFromRegistry() {

	m_vFilepaths = m_pSettings->getMultiSZAsVector(L"RecentFiles");
}

void RecentFiles::SaveRecentFilesInRegistry() {

	m_pSettings->setMultiSZ(L"RecentFiles", m_vFilepaths);
}

bool RecentFiles::IsFilepathAlreadyInList(const WCHAR * szFilepath) {

	wstring filepath{ szFilepath };

	auto found = find_if(begin(m_vFilepaths), end(m_vFilepaths),
		[&filepath](wstring const & contained_filepath) -> bool {
		return filepath == contained_filepath;
	});

	return found != end(m_vFilepaths);
}

void RecentFiles::FileOpened(const WCHAR * szFilepath) {

	if( !IsFilepathAlreadyInList(szFilepath) ) {

		m_vFilepaths.push_back(szFilepath);

		m_fChanged = true;

		while( m_vFilepaths.size() > 5 ) {
			RemoveRecentFile(0);
		}

		UpdateRecentFilesMenu();
	}

}

void RecentFiles::RemoveRecentFile(UINT index) {

	if( index >= m_vFilepaths.size() )	return;

	m_vFilepaths.erase(begin(m_vFilepaths) + index);
	
	this->m_fChanged = true;

	UpdateRecentFilesMenu();
}

HMENU RecentFiles::CreateRecentFilesMenu() {

	auto hRecentFiles = CreatePopupMenu();
	return hRecentFiles;
}

void RecentFiles::DestroyRecentFilesMenu() {

	if( m_hRecentFiles )	DestroyMenu(m_hRecentFiles);
}

void RecentFiles::UpdateRecentFilesMenu() {

	HMENU hRecentFiles = GetRecentFilesMenu();
	ClearMenu(hRecentFiles);

	const auto& vRecentFiles = GetRecentFiles();
	BOOL ret;
	if( vRecentFiles.size() > 0 ) {
		ULONG menuID = RECENT_FILE_MENU_ID;
		

		MENUITEMINFO mii{ sizeof(MENUITEMINFO), 0 };
		mii.fMask = MIIM_STRING | MIIM_ID;

		int index{ 0 };
		for_each(begin(vRecentFiles), end(vRecentFiles),
			[hRecentFiles, &menuID, &ret, &index, &mii](wstring const & filepath) {

			++index;
			wstring caption = L"&" + std::to_wstring(index) + L"  " + filepath.c_str();
			mii.dwTypeData = (LPWSTR) caption.c_str();

			mii.wID = menuID++;
			mii.cch = wcslen(caption.c_str());

			ret = InsertMenuItem(hRecentFiles, index - 1, TRUE, &mii);
		});

		wstring caption{ L"&Clear" };
		mii.wID = RECENT_FILE_MENU_CLEAR;
		mii.dwTypeData = (LPWSTR) caption.c_str();
		mii.cch = wcslen(caption.c_str());

		ret = InsertMenuItem(hRecentFiles, index, TRUE, &mii);
	}
}

void RecentFiles::ClearMenu(HMENU hMenu) {

	if( !hMenu )	return;

	auto count = GetMenuItemCount(hMenu);
	BOOL ret;
	for( ; count > 0; --count ) {

		ret = RemoveMenu(hMenu, count - 1, MF_BYPOSITION);
	}

}
