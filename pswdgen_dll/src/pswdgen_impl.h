#pragma once

#include <Windows.h>

#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>

#include <unordered_map>
#include <utility>

#include "pswdgen_h.h"

class ATL_NO_VTABLE PasswordGeneratorImpl :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<PasswordGeneratorImpl, &CLSID_PasswordGenerator>,
	public IDispatchImpl<IPasswordGenerator, &IID_IPasswordGenerator, &LIBID_PasswordGenerator> {

	BEGIN_COM_MAP(PasswordGeneratorImpl)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IPasswordGenerator)
	END_COM_MAP()

	PasswordGeneratorImpl();
	~PasswordGeneratorImpl();

	DECLARE_NO_REGISTRY()

	DECLARE_NOT_AGGREGATABLE(PasswordGeneratorImpl)

	STDMETHODIMP GetProperty(BSTR propertyname, BSTR * propertyvalue);
	STDMETHODIMP SetProperty(BSTR propertyname, BSTR propertyvalue, VARIANT_BOOL * pres);

	STDMETHODIMP GeneratePassword(BSTR * pbstr_password);


	using property_pair = std::pair<std::wstring, std::wstring>;

	std::unordered_map<std::wstring, std::wstring> m_properties = {
		property_pair{L"c_lower_case", L"4"},
		property_pair{L"c_upper_case", L"2"},
		property_pair{L"c_digits", L"4"},
		property_pair{L"c_symbols", L"1"},
		property_pair{L"c_spaces", L"0"},
		property_pair{L"symbol_values", L"%!*"}
	};
};

OBJECT_ENTRY_AUTO(CLSID_PasswordGenerator, PasswordGeneratorImpl)