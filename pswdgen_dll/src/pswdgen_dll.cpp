#include <Windows.h>
#include <windowsx.h>

#include "pswdgen_h.h"
#include "pswdgen_impl.h"

struct CAtlPasswordGeneratorModule : public CAtlDllModuleT<CAtlPasswordGeneratorModule> {

	DECLARE_LIBID(LIBID_PasswordGenerator)
};

CAtlPasswordGeneratorModule _AtlModule;

HMODULE			g_hModule;	

BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, void * pv) {

	switch( reason ) {

	case DLL_PROCESS_ATTACH:

		g_hModule = hModule;

		break;

	}

	return _AtlModule.DllMain(reason, pv);
}

HRESULT CALLBACK DllGetClassObject(REFCLSID objGuid, REFIID factoryGuid, void ** ppv) {

	return _AtlModule.DllGetClassObject(objGuid, factoryGuid, ppv);

}

HRESULT CALLBACK DllCanUnloadNow() {

	return _AtlModule.DllCanUnloadNow();

}

