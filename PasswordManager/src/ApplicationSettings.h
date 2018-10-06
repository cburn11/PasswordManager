#pragma once

#include <Windows.h>

#include <atlcomcli.h>

#include <string>
#include <unordered_map>

class ApplicationSettings {

public:

	ApplicationSettings(const WCHAR * szAppSubKey);
	~ApplicationSettings();

	DWORD getDWORD(const WCHAR * szKey);
	DWORD setDWORD(const WCHAR * szKey, DWORD value);

	const BSTR getSZ(const WCHAR * szKey);
	bool setSZ(const WCHAR * szKey, const BSTR szValue);

	const BSTR getMultiSZ(const WCHAR * szKey);
	bool setMultiSZ(const WCHAR * szKey, const BSTR szValue);

private:

	class RegVariant {

	public:

		RegVariant(const WCHAR * szRegKey, const WCHAR * szKey, DWORD type);
		RegVariant(const WCHAR * szRegKey, const WCHAR * szKey, VARIANT * pvar);

		RegVariant(RegVariant&& rv);

		~RegVariant();

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
	};

	std::unordered_map<std::wstring, RegVariant> kv_pairs;

	std::wstring m_AppSubKey;

	bool EnumKeyValues();

	VARIANT getValue(const WCHAR * szKey);
	VARIANT setValue(const WCHAR * szKey, VARIANT * pvarValue);

};