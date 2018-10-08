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

	}

}

void RecentFiles::RemoveRecentFile(UINT index) {

	if( index >= m_vFilepaths.size() )	return;

	m_vFilepaths.erase(begin(m_vFilepaths) + index);
	
	this->m_fChanged = true;

}
