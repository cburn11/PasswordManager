#include <Windows.h>

#include "ApplicationImpl.h"
#include "pswdgen_h.h"

#include "pswdgen_Application.h"

#include "iunknownmacros.h"

STDMETHODIMP pswdgen_Application::get_Visible(VARIANT_BOOL* pvis) {

	auto vis = IsWindowVisible(m_hwndMainWIndow);

	if( vis )
		*pvis = VARIANT_TRUE;
	else
		*pvis = VARIANT_FALSE;

	return S_OK;
}

STDMETHODIMP pswdgen_Application::put_Visible(VARIANT_BOOL vis) {

	if( VARIANT_TRUE == vis )
		ShowWindow(m_hwndMainWIndow, SW_NORMAL);
	else
		ShowWindow(m_hwndMainWIndow, SW_HIDE);

	return S_OK;
}

STDMETHODIMP pswdgen_Application::Quit() {

	DestroyWindow(m_hwndMainWIndow);

	return S_OK;
}

STDMETHODIMP pswdgen_Application::GetProperty(BSTR propertyname, BSTR* propertyvalue) {

	auto ret = SendMessage(m_hwndMainWIndow, WM_APP_GET_PROPERTY, 
		( WPARAM ) propertyname, ( LPARAM ) propertyvalue);

	return ret == 0 ? S_OK : E_FAIL;

}

STDMETHODIMP pswdgen_Application::SetProperty(BSTR propertyname, BSTR propertyvalue, VARIANT_BOOL* pres) {

	auto ret = SendMessage(m_hwndMainWIndow, WM_APP_SET_PROPERTY,
		( WPARAM ) propertyname, ( LPARAM ) propertyvalue);

	if( 0 == ret )
		*pres = VARIANT_TRUE;
	else
		*pres = VARIANT_FALSE;

	return S_OK;
}

STDMETHODIMP pswdgen_Application::GeneratePassword(BSTR* pbstr_password) {
	
	*pbstr_password = nullptr;

	BSTR bstrPassword = nullptr;
	SendMessage(m_hwndMainWIndow, WM_APP_GENERATE_PASSWORD, 0, ( LPARAM ) &bstrPassword);
	*pbstr_password = bstrPassword;

	return (bstrPassword) ? S_OK : E_FAIL;
}

class CApplication : public CAtlExeModuleT< CApplication > {

public:
	
	DECLARE_LIBID(LIBID_PasswordGenerator)


};

CApplication _AtlModule;



class Application_ClassFactory : public IClassFactory {

public:
	
	HWND m_hwnd;
	DWORD m_dwRegister;

	IPasswordGenerator* m_p_pwgen;

	BEGIN_INTERFACE_TABLE(Application_ClassFactory)
		IMPLEMENTS_INTERFACE(IClassFactory)
	END_INTERFACE_TABLE

	IMPLEMENT_UNKNOWN_NO_DELETE(Application_ClassFactory)

	STDMETHODIMP CreateInstance(IUnknown* punk, REFIID riid, void** ppvObject) {

		HRESULT hr;

		*ppvObject = nullptr;

		if( punk )
			return CLASS_E_NOAGGREGATION;

		if( riid == IID_IUnknown ||
			riid == IID_IDispatch ||
			riid == IID_IApplication ) {

			CComObject<pswdgen_Application> * papp = new CComObject<pswdgen_Application>{};
			if( papp != nullptr ) {
				papp->setHwnd(m_hwnd);
				papp->setPasswordGenerator(m_p_pwgen);
				hr = papp->QueryInterface(riid, ppvObject);
				return hr;
			}
		}

		return E_NOINTERFACE;
	}

	STDMETHODIMP LockServer(BOOL fLock) {

		return S_OK;
	}
};

Application_ClassFactory g_cf{};

extern "C" void* CreateApplication(HWND hwnd, IPasswordGenerator * p_pwgen) {

	HRESULT hr;


	IUnknown * punk;
	hr = g_cf.QueryInterface(IID_PPV_ARGS(&punk));
		
	g_cf.m_hwnd = hwnd;
	g_cf.m_p_pwgen = p_pwgen;

	hr = CoRegisterClassObject(CLSID_Application, punk, CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &g_cf.m_dwRegister);	

	return &g_cf;
}

extern "C" void DeleteApplication(void* papp) {

	HRESULT hr;

	if( papp == &g_cf ) {
		hr = CoRevokeClassObject(g_cf.m_dwRegister);
	}
}