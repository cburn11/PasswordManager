#pragma once

#include <Windows.h>

#include <atlcomcli.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

class ApplicationSettings {

public:

	ApplicationSettings(const WCHAR * szAppSubKey);
	~ApplicationSettings();

	DWORD getDWORD(const WCHAR * szKey);
	DWORD setDWORD(const WCHAR * szKey, DWORD value);

	const BSTR getSZ(const WCHAR * szKey);
	bool setSZ(const WCHAR * szKey, const BSTR szValue);

	const BSTR getMultiSZ(const WCHAR * szKey);
	std::vector<std::wstring> getMultiSZAsVector(const WCHAR * szKey);
	bool setMultiSZ(const WCHAR * szKey, const BSTR szValue);
	bool setMultiSZ(const WCHAR * szKey, const std::vector<std::wstring>& values);

	std::pair<const BYTE *, size_t> getBytes(const WCHAR * szKey);
	bool setBytes(const WCHAR * szKey, const BYTE * pBuffer, size_t cb);
	bool setBytes(const WCHAR * szKey, const std::pair<const BYTE *, size_t>& byte_pair);

	bool contains(const WCHAR * szKey);
	VARTYPE getType(const WCHAR * szKey);

	bool clear(const WCHAR * szKey);

private:

	struct excpNoApplicationSubkey {};

	class RegVariant;

	std::unordered_map<std::wstring, RegVariant> kv_pairs;

	std::wstring m_AppSubKey;

	bool EnumKeyValues();

	VARIANT getValue(const WCHAR * szKey);
	VARIANT setValue(const WCHAR * szKey, VARIANT * pvarValue);

	void CreateApplicationSubKey();
};

class ApplicationSettings::RegVariant {

public:

	RegVariant(const WCHAR * szRegKey, const WCHAR * szKey, DWORD type);
	RegVariant(const WCHAR * szRegKey, const WCHAR * szKey, VARIANT * pvar);

	RegVariant(RegVariant&& rv);

	~RegVariant();

	void SetMultiSZ() { m_fMultiSZ = true; }
	void ClearMultiSZ() { m_fMultiSZ = false; }

	size_t getBinarySize() { return m_cbBin; }
	void setBinarySize(size_t cb) { m_cbBin = cb; }

	operator VARIANT() { return m_var; }

	// Attach frees any memory currently held, and then makes a copy of the src structure.
	// Which means Attach takes ownership of any memory (bstr) to whichsrcVar points 
	void operator=(VARIANT * pvar) { m_var.Attach(pvar); m_fNeedToSave = true; }

private:

	std::wstring m_key;
	std::wstring m_RegKey;

	CComVariant m_var{ VT_EMPTY };

	bool m_fNeedToSave{ false };
	bool m_fMultiSZ{ false };

	bool SaveToRegistry();

	size_t m_cbBin{ 0 };
};