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

	HKEY hRecentFilesKey;

	auto lRes = RegOpenKey(HKEY_CURRENT_USER, APPLICATION_SUBKEY, &hRecentFilesKey);
	if( lRes == ERROR_SUCCESS ) {

		DWORD type, cbStr;
		lRes = RegGetValue(hRecentFilesKey, nullptr, L"RecentFiles", RRF_RT_REG_MULTI_SZ,
			&type, nullptr, &cbStr);
		if( lRes == ERROR_SUCCESS ) {
			WCHAR * szRecentFiles = 	// Double /0 terminated string
				( WCHAR * ) new WCHAR[cbStr]{ 0 };
			WCHAR * szOrig = szRecentFiles;
			lRes = RegGetValue(hRecentFilesKey, nullptr, L"RecentFiles", RRF_RT_REG_MULTI_SZ,
				&type, szRecentFiles, &cbStr);
			if( lRes == ERROR_SUCCESS ) {
				auto cch = wcslen(szRecentFiles);
				while( cch > 0 ) {
					m_vFilepaths.push_back(szRecentFiles);
					szRecentFiles += cch + 1;
					cch = wcslen(szRecentFiles);
				}
			}

			delete[] szOrig;

		} 

		RegCloseKey(hRecentFilesKey);

	} else if( ERROR_FILE_NOT_FOUND == lRes ) {

		//	Create Recent files for current user

		HKEY hPackageKey;
		lRes = RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Clifford", &hPackageKey);

		RegCloseKey(hPackageKey);

		lRes = RegCreateKey(HKEY_CURRENT_USER, APPLICATION_SUBKEY, &hRecentFilesKey);

		RegCloseKey(hRecentFilesKey);
	}
}

void RecentFiles::SaveRecentFilesInRegistry() {
	HKEY hRecentFilesKey;

	auto lRes = RegOpenKey(HKEY_CURRENT_USER, APPLICATION_SUBKEY, &hRecentFilesKey);
	if( lRes == ERROR_SUCCESS ) {

		ULONG size = 0;
		for_each(begin(m_vFilepaths), end(m_vFilepaths),
			[&size](wstring const & file) {
			size += wcslen(file.c_str()) + 1;
		});

		WCHAR * dsz = ( WCHAR * ) new WCHAR[size + 1]{ 0 };
		if( dsz ) {
			size = 0;
			for_each(begin(m_vFilepaths), end(m_vFilepaths),
				[&size, dsz](wstring const & filepath) {
				size += wsprintf(dsz + size, L"%s", filepath.c_str());
				++size;
			});

			lRes = RegSetValueEx(hRecentFilesKey, L"RecentFiles", 0, REG_MULTI_SZ, (BYTE *) dsz, size * sizeof(*dsz));
			delete[] dsz;
		}

		RegCloseKey(hRecentFilesKey);
	}
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

	m_vFilepaths.erase(begin(m_vFilepaths) + index);
	
	this->m_fChanged = true;

}
