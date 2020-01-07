#include "Helper.h"

#include <regex>
#include <memory>

std::wstring SanitizeCommandLine(const WCHAR * szCmdLine) {

	std::wstring cmdline{ szCmdLine };

	using std::wregex;
	using std::wsmatch;
	using std::regex_search;

	wregex	RegExp{ LR"(([A-Z]|[a-z]|:|\\|\.|\s|\d|-|\(|\))+)" };
	wsmatch	match;

	auto res = regex_search(cmdline, match, RegExp);
	if( res )
		return std::wstring{ match[0].first, match[0].second };
	else
		return L"";

}

std::wstring GetRegSZValue(HKEY hkeyParent, const WCHAR * szSubKey, const WCHAR * name) {

	std::wstring value{ L"" };
	std::unique_ptr<WCHAR> pszValue;
	HKEY hkeyValue = NULL;
	DWORD cbValue;
	LSTATUS res;

	if( !hkeyParent )				goto _ERROR;

	res = RegOpenKey(hkeyParent, szSubKey, &hkeyValue);
	if( ERROR_SUCCESS != res )		goto _ERROR;

	res = RegQueryValueEx(hkeyValue, name, NULL, NULL, NULL, &cbValue);
	if( ERROR_SUCCESS != res ||
		cbValue == 0 )				goto _ERROR;

	pszValue.reset(new WCHAR[cbValue]{ 0 });
	if( !pszValue )					goto _ERROR;

	res = RegQueryValueEx(hkeyValue, name, NULL, NULL, (LPBYTE) pszValue.get(), &cbValue);
	if( ERROR_SUCCESS != res )		goto _ERROR;

	value = pszValue.get();

_ERROR:

	if( hkeyValue != NULL )		RegCloseKey(hkeyValue);

	return value;
}

std::wstring GetFilenameFromPath(const std::wstring& path) {

	auto offset = path.find_last_of(L'\\');

	if( offset == std::wstring::npos )	return L"";

	return path.substr(offset + 1, path.size() - offset);
}

std::wstring GetParentFromFilepath(const std::wstring& path) {

	auto offset = path.find_last_of(L'\\');

	if( offset == std::wstring::npos )	return L"";

	return path.substr(0, offset);
}