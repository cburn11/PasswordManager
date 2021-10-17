#include <Windows.h>

#include "ApplicationImpl.h"
#include "pswdgen_h.h"
#include "resource.h"
#include "pswdgen_Application.h"
#include "iunknownmacros.h"

extern "C" INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
extern "C" HINSTANCE g_hInstance;
extern "C" HWND g_hwndMain;

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
				SetWindowLongPtr(m_hwnd, GWLP_USERDATA, ( LONG_PTR ) papp);
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

extern "C" void* CreateApplication(IPasswordGenerator * p_pwgen) {

	HRESULT hr;


	IUnknown * punk;
	hr = g_cf.QueryInterface(IID_PPV_ARGS(&punk));
		
	g_cf.m_hwnd = g_hwndMain;
	g_cf.m_p_pwgen = p_pwgen;

	hr = CoRegisterClassObject(CLSID_Application, punk, CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE, &g_cf.m_dwRegister);	

	return &g_cf;
}

extern "C" void DeleteApplication(void* papp) {

	HRESULT hr;

	if( papp == &g_cf ) {
		hr = CoRevokeClassObject(g_cf.m_dwRegister);
	}
}

HRESULT AutoInvoke(REFGUID _LIB, REFIID _IID, int type, VARIANT* pvResult, IDispatch* pDispIn,
	OLECHAR* szName, int cArgs, ...) {

	HRESULT			hr;

	DISPPARAMS		dispparamsAuto;
	EXCEPINFO		excepinfoAuto;
	UINT			uArgErrAuto;

	VARIANT* pvAuto = NULL;

	DISPID			dispidSave = DISPID_PROPERTYPUT;
	DISPID			dispidAuto;

	va_list			marker = nullptr;

	LPTYPELIB		pTypeLib = nullptr;
	hr = LoadRegTypeLib(_LIB, 1, 0, 0, &pTypeLib);

	ITypeInfo* pTypeInfo = nullptr;
	hr = pTypeLib->GetTypeInfoOfGuid(_IID, &pTypeInfo);

	VARIANT* pvar = NULL;
	int			i;

	if( cArgs > 0 ) {

		va_start(marker, cArgs);

		pvAuto = ( VARIANT* ) malloc(sizeof(VARIANT) * cArgs);

		if( !pvAuto ) {

			hr = E_OUTOFMEMORY;

			goto CLEANUP;

		}

	}

	for( i = 0; i < cArgs; ++i ) {

		pvar = va_arg(marker, VARIANT*);

		memcpy((pvAuto + i), pvar, sizeof(VARIANT));

	}

	hr = DispGetIDsOfNames(pTypeInfo, &szName, 1, &dispidAuto);

	if( FAILED(hr) ) {

		pvResult = NULL;

		goto CLEANUP;

	}

	dispparamsAuto.cArgs = cArgs;
	dispparamsAuto.cNamedArgs = 0;
	dispparamsAuto.rgvarg = pvAuto;
	dispparamsAuto.rgdispidNamedArgs = NULL;

	if( type & DISPATCH_PROPERTYPUT ) {

		dispparamsAuto.cNamedArgs = 1;
		dispparamsAuto.rgdispidNamedArgs = &dispidSave;

	}

	hr = pDispIn->Invoke(dispidAuto, IID_NULL, LOCALE_SYSTEM_DEFAULT, type,
		&dispparamsAuto, pvResult, &excepinfoAuto, &uArgErrAuto);

	va_end(marker);

CLEANUP:

	if( pvAuto )	free(pvAuto);
	if( pTypeInfo )	pTypeInfo->Release();
	if( pTypeLib )	pTypeLib->Release();

	return hr;
}

extern "C" void TriggerQuit(void* p_v_app) {

	if( !p_v_app )	return;

	CComObject<pswdgen_Application>* p_app = (CComObject<pswdgen_Application> *) p_v_app;

	HRESULT hr;

	IConnectionPoint* p_cp;
	hr = p_app->FindConnectionPoint(__uuidof(IApplicationEvents), &p_cp);

	if( S_OK == hr ) {

		IEnumConnections* p_enums;

		hr = p_cp->EnumConnections(&p_enums);
		if( S_OK == hr ) {

			CONNECTDATA cd{ 0 };
			while( S_OK == p_enums->Next(1, &cd, nullptr) ) {
				IDispatch* p_disp;
				hr = cd.pUnk->QueryInterface(IID_PPV_ARGS(&p_disp));
				if( S_OK == hr ) {
					VARIANT varResult;
					hr = AutoInvoke(LIBID_PasswordGenerator, IID_IApplicationEvents,
						DISPATCH_METHOD, &varResult, p_disp, L"OnQuit", 0);

					p_disp->Release();
				}
			}

			p_enums->Release();
		}

		p_cp->Release();
	}
}

extern "C" void TriggerPropertyChange(void* p_v_app, BSTR name, BSTR value) {

	if( !p_v_app )	return;

	CComObject<pswdgen_Application>* p_app = (CComObject<pswdgen_Application> *) p_v_app;

	HRESULT hr;

	IConnectionPoint* p_cp;
	hr = p_app->FindConnectionPoint(__uuidof(IApplicationEvents), &p_cp);

	if( S_OK == hr ) {

		IEnumConnections* p_enums;

		hr = p_cp->EnumConnections(&p_enums);
		if( S_OK == hr ) {

			CONNECTDATA cd{ 0 };
			while( S_OK == p_enums->Next(1, &cd, nullptr) ) {
				IDispatch* p_disp;
				hr = cd.pUnk->QueryInterface(IID_PPV_ARGS(&p_disp));
				if( S_OK == hr ) {
					VARIANT varResult;
					VARIANT varName, varValue;
					varName.vt = VT_BSTR;	varName.bstrVal = name;
					varValue.vt = VT_BSTR;	varValue.bstrVal = value;
					hr = AutoInvoke(LIBID_PasswordGenerator, IID_IApplicationEvents,
						DISPATCH_METHOD, &varResult, p_disp, L"PropertyChange", 2, &varValue, &varName);

					p_disp->Release();
				}
			}

			p_enums->Release();
		}

		p_cp->Release();
	}
}

void TriggerPasswordGenerated(void* p_v_app, BSTR password) {
	if( !p_v_app )	return;

	CComObject<pswdgen_Application>* p_app = (CComObject<pswdgen_Application> *) p_v_app;

	HRESULT hr;

	IConnectionPoint* p_cp;
	hr = p_app->FindConnectionPoint(__uuidof(IApplicationEvents), &p_cp);

	if( S_OK == hr ) {

		IEnumConnections* p_enums;

		hr = p_cp->EnumConnections(&p_enums);
		if( S_OK == hr ) {

			CONNECTDATA cd{ 0 };
			while( S_OK == p_enums->Next(1, &cd, nullptr) ) {
				IDispatch* p_disp;
				hr = cd.pUnk->QueryInterface(IID_PPV_ARGS(&p_disp));
				if( S_OK == hr ) {
					VARIANT varResult;
					VARIANT varPassword;
					varPassword.vt = VT_BSTR;	varPassword.bstrVal = password;
					hr = AutoInvoke(LIBID_PasswordGenerator, IID_IApplicationEvents,
						DISPATCH_METHOD, &varResult, p_disp, L"PasswordGenerated", 1, &varPassword);

					p_disp->Release();
				}
			}

			p_enums->Release();
		}

		p_cp->Release();
	}
}