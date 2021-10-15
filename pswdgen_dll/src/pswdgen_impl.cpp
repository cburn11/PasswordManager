#include <Windows.h>

#include <unordered_map>
#include <utility>
#include <string>
#include <algorithm>

#include "pswdgen_impl.h"
#include "rndnum.h"

PasswordGeneratorImpl::PasswordGeneratorImpl() {



}

PasswordGeneratorImpl::~PasswordGeneratorImpl() {


}


STDMETHODIMP PasswordGeneratorImpl::GetProperty(BSTR propertyname, BSTR* propertyvalue) {

	if( propertyvalue == nullptr )
		return E_POINTER;

	*propertyvalue = nullptr;

	std::wstring property_name{ propertyname };
	if( property_name == L"" )
		return E_INVALIDARG;

	try {
		auto value = m_properties.at(property_name);
		*propertyvalue = SysAllocString(value.c_str());
	} catch( std::exception e ) {
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP PasswordGeneratorImpl::SetProperty(BSTR propertyname, BSTR propertyvalue, VARIANT_BOOL * pres) {

	if( pres )	*pres = VARIANT_FALSE;

	std::wstring prop_value{ propertyvalue };
	std::wstring prop_name{ propertyname };

	if( prop_name == L"" || prop_value == L"" )
		return E_INVALIDARG;

	try {
		auto& value_ref = m_properties.at(prop_name);
		value_ref = prop_value;
		if( pres )	*pres = VARIANT_TRUE;
	} catch( std::exception e ) {
		return E_INVALIDARG;
	}

	return S_OK;
}

STDMETHODIMP PasswordGeneratorImpl::GeneratePassword(BSTR * pbstr_password) {

	if( pbstr_password == nullptr )
		return E_POINTER;

	std::wstring lower{ L"abcdefghijklmnopqrstuvwxyz" };
	std::wstring upper{ lower };
	std::transform(std::begin(upper), std::end(upper), std::begin(upper), toupper);
	std::wstring digits{ L"0123456789" };
	std::wstring symbols = m_properties.at(L"symbol_values");
	std::wstring space{ L" " };

	std::wstring* values[5] = {
		&lower, &upper, &digits, &symbols, &space
	};

	unsigned int counts[5] = { 0 };

	counts[0] = std::stoi(m_properties.at(L"c_lower_case").c_str());
	unsigned int& cLower = counts[0];

	counts[1] = std::stoi(m_properties.at(L"c_upper_case").c_str());
	unsigned int& cUpper = counts[1];

	counts[2] = std::stoi(m_properties.at(L"c_digits").c_str());
	unsigned int& cDigits = counts[2];

	counts[3] = std::stoi(m_properties.at(L"c_symbols").c_str());
	unsigned int& cSymbols = counts[3];

	counts[4] = std::stoi(m_properties.at(L"c_spaces").c_str());
	unsigned int& cSpaces = counts[4];

	unsigned int cchPassword = cLower + cUpper + cDigits + cSymbols + cSpaces;

	auto szPassword = SysAllocStringLen(nullptr, cchPassword);
	*pbstr_password = szPassword;
	if( szPassword == nullptr )
		return E_OUTOFMEMORY;

	for( unsigned int index = 0; index < cchPassword; ++index ) {

		wchar_t c;
		int type = rndint(0, 4);

		if( counts[type] < 1 ) {
			--index;
			continue;
		}

		counts[type] -= 1;

		switch( type ) {

		case 0:
		case 1:
		case 2:
		case 3: {

			int size = values[type]->size();
			int char_index = rndint(0, size - 1);

			c = values[type]->at(char_index);

			break; }

		case 4:	{

			c = L' ';
			
			if( index == 0 ) {
				counts[type] += 1;
				--index;
				continue;
			}

			if( index == cchPassword - 1 ) {
				c = *(szPassword + index - 1);
				*(szPassword + index - 1) = L' ';
			}

			break; }
		}

		*(szPassword + index) = c;
	}

	return S_OK;
}
