#include "eventsink.h"
#include "PasswordManager.h"

#include <string>

CAtlApplicationEventModule _AtlModule;

STDMETHODIMP ApplicationEventsImpl_ATL::OnQuit() {

	return S_OK;
}

STDMETHODIMP ApplicationEventsImpl_ATL::PropertyChange(BSTR bstrName, BSTR bstrValue) {

	std::wstring name{ bstrName };
	std::wstring str_value{ bstrValue };

	auto value = std::stoi(str_value);

	UserData* pUserData = ( UserData* ) GetWindowLongPtr(m_hwndApp, GWLP_USERDATA);

	if( name == L"c_lower_case" ||
		name == L"c_upper_case" ||
		name == L"c_digits" ||
		name == L"c_symbols" ||
		name == L"c_spaces" ) {
			
		pUserData->pSettings->setDWORD(bstrName, value);

	}

	return S_OK;
}