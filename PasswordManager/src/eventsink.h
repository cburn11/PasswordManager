#pragma once

#include <Windows.h>

#include <atlbase.h>
#include <atlcom.h>

#include "pswdgen_h.h"

class ApplicationEventsImpl_ATL :
	public IDispEventImpl<1, ApplicationEventsImpl_ATL, &IID_IApplicationEvents, &LIBID_PasswordGenerator, 1, 0> {

	HWND	m_hwndApp;

public:

	BEGIN_SINK_MAP(ApplicationEventsImpl_ATL)
		SINK_ENTRY_EX(1, IID_IApplicationEvents, 0x30000010, ApplicationEventsImpl_ATL::PropertyChange)
		SINK_ENTRY_EX(1, IID_IApplicationEvents, 0x30000030, ApplicationEventsImpl_ATL::OnQuit)
	END_SINK_MAP()

	STDMETHODIMP OnQuit();
	STDMETHODIMP PropertyChange(BSTR name, BSTR value);

	ApplicationEventsImpl_ATL(HWND hwnd) : m_hwndApp{ hwnd } {

	}
};

struct CAtlApplicationEventModule : public CAtlModuleT<CAtlApplicationEventModule> {

};

extern CAtlApplicationEventModule _AtlModule;