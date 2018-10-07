#include <Windows.h>

#include "ApplicationSettings.h"


ApplicationSettings::ApplicationSettings(const WCHAR * szAppSubKey) : m_AppSubKey{ szAppSubKey } {
	
	if( !szAppSubKey ) throw std::exception{"nullptr"};

	if( wcslen(szAppSubKey) < 1 ) throw std::exception{"empty key"};
	
	try {
		EnumKeyValues();
	} catch(excpNoApplicationSubkey){
		CreateApplicationSubKey();
	}
}

ApplicationSettings::~ApplicationSettings() {

}

void ApplicationSettings::CreateApplicationSubKey() {

	HKEY hPackageKey, hApplicationKey;
	auto lRes = RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Clifford", &hPackageKey);

	RegCloseKey(hPackageKey);

	lRes = RegCreateKey(HKEY_CURRENT_USER, m_AppSubKey.c_str(), &hApplicationKey);
	RegCloseKey(hApplicationKey);
}

bool ApplicationSettings::EnumKeyValues() {

	LSTATUS res;
	HKEY hKey;
	res = RegOpenKey(HKEY_CURRENT_USER, m_AppSubKey.c_str(), &hKey);
	if( ERROR_SUCCESS != res )		throw excpNoApplicationSubkey{};

	constexpr UINT cchMax = MAXSHORT;
	DWORD cchName = cchMax;
	WCHAR szName[cchMax]{ 0 };
	int i = 0;
	DWORD type;

	while( ( res = RegEnumValue(hKey, i++, szName, &cchName, NULL, &type, NULL, NULL) ) != ERROR_NO_MORE_ITEMS ) {

		auto pair = std::make_pair(
			std::wstring{ szName }, 
			RegVariant{ m_AppSubKey.c_str(), szName, type }
		);

		kv_pairs.insert(std::move(pair));

		cchName = cchMax;
	}

	RegCloseKey(hKey);

	return true;
}

VARIANT ApplicationSettings::getValue(const WCHAR * szKey) {

	if( !szKey )	return VARIANT{ VT_ERROR };

	if( wcslen(szKey) < 1 )	return VARIANT{ VT_ERROR };

	VARIANT varReturn{ VT_EMPTY };

	try {

		varReturn = kv_pairs.at(szKey);
	
	} catch( std::exception e ) {

#ifdef _DEBUG
		std::wstring out{ L"Key not present in applicationsettings: " };
		out += szKey;
		out += L"\n";

		OutputDebugString(out.c_str());
#endif // _NDEBUG

	}

	return varReturn;
}

DWORD ApplicationSettings::getDWORD(const WCHAR * szKey) {

	VARIANT varValue = getValue(szKey);

	if( VT_EMPTY == varValue.vt || VT_ERROR == varValue.vt )
		return 0;

	return varValue.ulVal;
}

VARIANT ApplicationSettings::setValue(const WCHAR * szKey, VARIANT * pvarValue) {

	if( !szKey )			return VARIANT{ VT_ERROR };

	if( wcslen(szKey) < 1 )	return VARIANT{ VT_ERROR };

	if( !pvarValue )		return VARIANT{ VT_ERROR };

	VARIANT varPrev = getValue(szKey);

	if( varPrev.vt == VT_EMPTY ) {
		
		auto kv_pair = std::make_pair(std::wstring{ szKey }, RegVariant{ m_AppSubKey.c_str(), szKey, pvarValue });

		kv_pairs.insert(std::move(kv_pair));
		
	} else {

		auto& var = kv_pairs.at(szKey);

		if( pvarValue->ulVal != ((VARIANT) var).ulVal )
			var = pvarValue;
	}

	return varPrev;
}

DWORD ApplicationSettings::setDWORD(const WCHAR * szKey, DWORD value) {

	VARIANT varNew{ VT_UI4 };
	varNew.ulVal = value;

	VARIANT varOld = setValue(szKey, &varNew);

	if( VT_ERROR == varOld.vt || VT_EMPTY == varOld.vt )
		return 0;

	return varOld.ulVal;
}

const BSTR ApplicationSettings::getSZ(const WCHAR * szKey) {

	VARIANT varValue = getValue(szKey);

	if( VT_EMPTY == varValue.vt || VT_ERROR == varValue.vt )
		return nullptr;

	return varValue.bstrVal;
}

bool ApplicationSettings::setSZ(const WCHAR * szKey, const BSTR szValue) {

	VARIANT varNew{ VT_BSTR };
	varNew.bstrVal = szValue;

	VARIANT varOld = setValue(szKey, &varNew);

	if( VT_ERROR == varOld.vt || VT_EMPTY == varOld.vt )
		return false;

	return true;
}

const BSTR ApplicationSettings::getMultiSZ(const WCHAR * szKey) {

	return getSZ(szKey);
}
bool ApplicationSettings::setMultiSZ(const WCHAR * szKey, const BSTR szValue) {

	auto prev = setSZ(szKey, szValue);

	if( false == prev ) {
		try {
			auto& rv = kv_pairs.at(szKey);
			rv.SetMultiSZ();	
		} catch( std::exception ) {
#ifdef DEBUG
			
			std::wstring out{ L"Error adding : " };
			out += szKey;
			out += L"\n";

			OutputDebugString(out.c_str());
#endif // DEBUG
		}
	}

	return prev;
}

ApplicationSettings::RegVariant::RegVariant(const WCHAR * szRegKey, const WCHAR * szKey, DWORD type) :
	m_RegKey{ szRegKey }, m_key{ szKey } {

	LSTATUS res;
	HKEY hKey;
	res = RegOpenKey(HKEY_CURRENT_USER, szRegKey, &hKey);
	if( ERROR_SUCCESS != res )		return;

	DWORD cb;

	switch( type ) {

	case REG_DWORD:

		m_var.vt = VT_UI4;

		res = RegGetValue(hKey, nullptr, szKey, RRF_RT_REG_DWORD,
			nullptr, &m_var.ulVal, &cb);

		break;

	case REG_MULTI_SZ:
		m_fMultiSZ = true;
		// FALL THROUGH

	case REG_SZ:
		
		type = ( type == REG_SZ ) ? RRF_RT_REG_SZ : RRF_RT_REG_MULTI_SZ;

		m_var.vt = VT_BSTR;

		res = RegGetValue(hKey, nullptr, szKey, type,			//	RegGetValue,  cb includes terminating null/nulls
			nullptr, nullptr, &cb);
		if( ERROR_SUCCESS != res )	return;

		cb -= sizeof(m_var.bstrVal[0]);							//	SysAllocStringByteLen allocates cb + space for terminating null
																
		m_var.bstrVal = SysAllocStringByteLen(nullptr, cb);	
		if( !m_var.bstrVal )			return;
#ifdef DEBUG
		memset(m_var.bstrVal, 0x55, cb);
#endif // DEBUG
		res = RegGetValue(hKey, nullptr, szKey, type,
			nullptr, m_var.bstrVal, &cb);

		break;
	}

	RegCloseKey(hKey);
}

ApplicationSettings::RegVariant::RegVariant(const WCHAR * szRegKey, const WCHAR * szKey, VARIANT * pvar) :
	m_RegKey{ szRegKey }, m_key{ szKey }, m_fNeedToSave{ true } {

	m_var.Attach(pvar);
}

ApplicationSettings::RegVariant::RegVariant(RegVariant&& rv) {

	m_key = std::move(rv.m_key);
	m_RegKey = std::move(rv.m_RegKey);

	m_fNeedToSave = rv.m_fNeedToSave;
	m_fMultiSZ = rv.m_fMultiSZ;

	rv.m_fNeedToSave = false;
	
	rv.m_var.Detach(&m_var);
}

ApplicationSettings::RegVariant::~RegVariant() {

	if( !m_fNeedToSave )	return;

	SaveToRegistry();
}

bool ApplicationSettings::RegVariant::SaveToRegistry() {

#ifdef DEBUG
	std::wstring out{ L"Saving: " };
	out += m_key;
	out += L"\n";

	OutputDebugString(out.c_str());
#endif // DEBUG

	LSTATUS res;
	HKEY hKey;
	res = RegOpenKey(HKEY_CURRENT_USER, m_RegKey.c_str(), &hKey);
	if( ERROR_SUCCESS != res )		return false;

	DWORD cb;

	switch( m_var.vt ) {

	case VT_UI4:
		cb = 4;
		res = RegSetValueEx(hKey, m_key.c_str(), 0, REG_DWORD, (BYTE *) &m_var.ulVal, cb);
		break;

	case VT_BSTR:
		cb = SysStringByteLen(m_var.bstrVal);		//	Returned value does not include terminating null
		cb += sizeof(m_var.bstrVal[0]);				//	RegSetValueEx expects cb to include the terminating null
		res = RegSetValueEx(hKey, m_key.c_str(), 0, m_fMultiSZ ? REG_MULTI_SZ : REG_SZ, (BYTE *) m_var.bstrVal, cb);
		break;
	}

	RegCloseKey(hKey);

	return true;
}