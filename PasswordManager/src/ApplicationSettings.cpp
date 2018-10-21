#include <Windows.h>

#include "ApplicationSettings.h"

#include <algorithm>

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
		
		auto kv_pair = std::make_pair(
			std::wstring{ szKey }, 
			RegVariant{ m_AppSubKey.c_str(), szKey, pvarValue }
		);

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

std::vector<std::wstring> ApplicationSettings::getMultiSZAsVector(const WCHAR * szKey) {

	auto bstr = getMultiSZ(szKey);

	std::vector<std::wstring> m_vStrings;

	if( bstr ) {

		auto cch = wcslen(bstr);
		
		while( cch > 0 ) {
			m_vStrings.push_back(bstr);
			bstr += cch + 1;
			cch = wcslen(bstr);
		}
	}

	return m_vStrings;
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

bool ApplicationSettings::setMultiSZ(const WCHAR * szKey, const std::vector<std::wstring>& values) {

	ULONG size = 0;
	std::for_each(std::begin(values), std::end(values),
		[&size](std::wstring const & file) {
		size += file.length() + 1;	// +1 for null
	});

	if( size == 0 )	// empty
		size = 1;

	auto dsz = SysAllocStringLen(nullptr, size);	//	SysAllocStringLen allocates size + 1 for the terminating null
													//	But a subsequent call to SysStringLen or SysStringByteLen will
													//	return size, not the allocated size to terminating null
													//	Don't call SysFreeString, ApplicationSettings now owns dsz			

	if( !dsz ) throw std::exception{ "SysAllocStringLen error" };

	size = 0;
	std::for_each(std::begin(values), std::end(values),
		[&size, dsz](std::wstring const & filepath) {
		size += wsprintf(dsz + size, L"%s", filepath.c_str());
		++size;
	});

	return setMultiSZ(szKey, dsz);
}

std::pair<const BYTE *, size_t> ApplicationSettings::getBytes(const WCHAR * szKey) {

	try {
		auto& rv = kv_pairs.at(szKey);
		const auto& var = (VARIANT) rv;

		return std::make_pair(var.pbVal, rv.getBinarySize());

	} catch( std::exception ) {

		return std::make_pair<const BYTE *, size_t>(nullptr, 0);
	}
}

bool ApplicationSettings::setBytes(const WCHAR * szKey, const BYTE * pBuffer, size_t cb) {

	VARIANT varNew{ VT_UI1 | VT_BYREF };
	
	varNew.pbVal = new BYTE[cb]{ 0 };
	if( !varNew.pbVal ) throw std::exception{ "allocation error" };

	memcpy_s(varNew.pbVal, cb, pBuffer, cb);

	auto varPrev = setValue(szKey, &varNew);
	if( varPrev.vt == ( VT_UI1 | VT_BYREF ) )
		delete[] varPrev.pbVal;

	try {
		auto& rv = kv_pairs.at(szKey);
		rv.setBinarySize(cb);
	} catch( std::exception ) {
		if( varNew.pbVal )	delete[] varNew.pbVal;
		throw std::exception{ "Unable to add binary key" };
	}

	return !( varPrev.vt == VT_EMPTY || varPrev.vt == VT_ERROR );
}

bool ApplicationSettings::setBytes(const WCHAR * szKey, const std::pair<const BYTE *, size_t>& byte_pair) {

	auto&[pBuffer, cb] = byte_pair;

	return setBytes(szKey, pBuffer, cb);
}

bool ApplicationSettings::contains(const WCHAR * szKey) {

	auto var = getValue(szKey);

	return !(var.vt == VT_ERROR || var.vt == VT_EMPTY);
}

VARTYPE ApplicationSettings::getType(const WCHAR * szKey) {

	auto var = getValue(szKey);
	return var.vt;
}

bool ApplicationSettings::clear(const WCHAR * szKey) {

	try {
		
		auto& reg_var = kv_pairs.at(szKey);
		
		VARIANT * pvar = &( (VARIANT) reg_var );

		return ( VariantClear(pvar) == S_OK );

	} catch( std::exception ) {

		return false;
	}
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

	case REG_BINARY:

		m_var.vt = VT_UI1 | VT_BYREF;

		res = RegGetValue(hKey, nullptr, szKey, RRF_RT_REG_BINARY,			
			nullptr, nullptr, &cb);
		if( ERROR_SUCCESS != res )	return;

		m_var.pbVal = new BYTE[cb]{ 0 };
		if( !m_var.pbVal )			return;
		m_cbBin = cb;
#ifdef DEBUG
		memset(m_var.pbVal, 0x55, cb);
#endif // DEBUG
		res = RegGetValue(hKey, nullptr, szKey, type,
			nullptr, m_var.pbVal, &cb);

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

	case (VT_UI1 | VT_BYREF):

		res = RegSetValueEx(hKey, m_key.c_str(), 0, REG_BINARY, m_var.pbVal, m_cbBin);

		break;

	case VT_EMPTY:

		res = RegDeleteValue(hKey, m_key.c_str());

		break;
	}

	RegCloseKey(hKey);

	return true;
}