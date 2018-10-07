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

	auto szRecentFiles = 	// Double /0 terminated string
		m_pSettings->getMultiSZ(L"RecentFiles");

	if( !szRecentFiles )	return;
		
	WCHAR * szOrig = szRecentFiles;
			
	auto cch = wcslen(szRecentFiles);
	while( cch > 0 ) {
		m_vFilepaths.push_back(szRecentFiles);
		szRecentFiles += cch + 1;
		cch = wcslen(szRecentFiles);
	}
}

void RecentFiles::SaveRecentFilesInRegistry() {

	ULONG size = 0;
	for_each(begin(m_vFilepaths), end(m_vFilepaths),
		[&size](wstring const & file) {
		size += file.length() + 1;	// +1 for null
	});

	if( size == 0 )	// empty
		size = 1;

	auto dsz = SysAllocStringLen(nullptr, size);	//	SysAllocStringLen allocates size + 1 for the terminating null
													//	But a subsequent call to SysStringLen or SysStringByteLen will
													//	return size, not the allocated size to terminating null
	//	Don't call SysFreeString, ApplicationSettings now owns dsz			

	if( dsz ) {
		memset(dsz, 0, size * sizeof(dsz[0]));
		size = 0;
		for_each(begin(m_vFilepaths), end(m_vFilepaths),
			[&size, dsz](wstring const & filepath) {
			size += wsprintf(dsz + size, L"%s", filepath.c_str());
			++size;
		});

		m_pSettings->setMultiSZ(L"RecentFiles", dsz);
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
