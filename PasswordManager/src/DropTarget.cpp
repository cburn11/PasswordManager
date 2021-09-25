#include <ShlObj.h>

#include "PasswordManager.h"
#include "DropTargetImpl.h"

DropTarget::DropTarget(HWND hwnd) : m_hwndMainWindow{ hwnd } {

	auto hr = OleInitialize(nullptr);
	
	m_cf_filename = RegisterClipboardFormat(CFSTR_FILENAME);
}

STDMETHODIMP DropTarget::QueryInterface(REFIID iid, void** punknown) {

	HRESULT hr = E_NOINTERFACE;
	*punknown = nullptr;

	if (iid == IID_IUnknown || iid == IID_IDropTarget) {
		*punknown = this;
		this->AddRef();
		hr = S_OK;
	}

	return hr;
}

STDMETHODIMP DropTarget::DragEnter(IDataObject* pDataObj, 
	DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {

	*pdwEffect = DROPEFFECT_NONE;

	HRESULT hr = this->InspectIDataObject(pDataObj);
	if (hr != S_OK)	return hr; 

	m_pCurrentDragDataObject = pDataObj;
	*pdwEffect = DROPEFFECT_COPY;

	return hr;
}

STDMETHODIMP DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
	
	if (m_pCurrentDragDataObject.p)
		*pdwEffect = DROPEFFECT_COPY;
	else
		*pdwEffect = DROPEFFECT_NONE;

	return S_OK;
}

STDMETHODIMP DropTarget::DragLeave(void) {

	m_pCurrentDragDataObject = nullptr;
	m_dragdrop_filepath = L"";

	return S_OK;
}

STDMETHODIMP DropTarget::Drop(IDataObject* pDataObj, 
	DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {

	if (m_pCurrentDragDataObject.p) {
		*pdwEffect = DROPEFFECT_COPY;
		PostMessage(m_hwndMainWindow, PM_DRAGDROPOPENFILE, 0, (LPARAM)m_dragdrop_filepath.c_str());
	} else {
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

HRESULT DropTarget::InspectIDataObject(IDataObject * pDataObject) {

	CComPtr<IDataObject> pCurrentDragDataObject{ pDataObject };
	IEnumFORMATETC* pEnumFmtEtc = nullptr;

	auto hr = pCurrentDragDataObject->EnumFormatEtc(DATADIR_GET, &pEnumFmtEtc);
	if (S_OK != hr) return E_INVALIDARG;

	FORMATETC fmt_etc{ 0 };
	ULONG cFetched = 0;

	while (S_OK == pEnumFmtEtc->Next(1, &fmt_etc, &cFetched)) {

		if (fmt_etc.cfFormat == m_cf_filename) {

			STGMEDIUM stgmedium{ 0 };

			hr = pCurrentDragDataObject->GetData(&fmt_etc, &stgmedium);
			if (S_OK == hr) {

				const wchar_t* p_str = (const wchar_t*) GlobalLock(stgmedium.hGlobal);
				if (p_str) {

					bool fCanOpen = false;

					SendMessage(m_hwndMainWindow, PM_QUERYOPENFILE, (WPARAM) p_str, (LPARAM) &fCanOpen);

					if (fCanOpen) {

						m_dragdrop_filepath = p_str;

						hr = S_OK;

					} else {

						this->DragLeave();

						hr = S_FALSE;
					}

					GlobalUnlock(stgmedium.hGlobal);
				}
			}
			break;
		}

	}

	return hr;
}