#pragma once

#include <Windows.h>

#include <atlbase.h>
#include <atlcom.h>

#include "pswdgen_h.h"

class pswdgen_Application :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IApplication, &IID_IApplication, &LIBID_PasswordGenerator, 1, 0>,
	public IConnectionPointContainerImpl<pswdgen_Application>,
	public IConnectionPointImpl<pswdgen_Application, &IID_IApplicationEvents> {

	HWND m_hwndMainWIndow;

	IPasswordGenerator* m_pPasswordGenerator;

public:

	DECLARE_NO_REGISTRY()

	DECLARE_NOT_AGGREGATABLE(pswdgen_Application)

	BEGIN_COM_MAP(pswdgen_Application)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IApplication)
		COM_INTERFACE_ENTRY(IConnectionPointContainer)
	END_COM_MAP()

	BEGIN_CONNECTION_POINT_MAP(pswdgen_Application)
		CONNECTION_POINT_ENTRY(IID_IApplicationEvents)
	END_CONNECTION_POINT_MAP()

	STDMETHODIMP get_Visible(VARIANT_BOOL* pvis);
	STDMETHODIMP put_Visible(VARIANT_BOOL vis);
	STDMETHODIMP Quit();

	STDMETHODIMP GetProperty(BSTR propertyname, BSTR* propertyvalue);
	STDMETHODIMP SetProperty(BSTR propertyname, BSTR propertyvalue, VARIANT_BOOL* pres);
	STDMETHODIMP GeneratePassword(BSTR* pbstr_password);

	DWORD m_dwRegister;

	void setHwnd(HWND hwnd) { this->m_hwndMainWIndow = hwnd; }
	void setPasswordGenerator(IPasswordGenerator* p) { m_pPasswordGenerator = p; }
};
