#pragma once

#include <windows.h>
#include <atlcomcli.h>

#include <string>

class DropTarget : public IDropTarget {

public:

	DropTarget(HWND hwnd);

	STDMETHODIMP QueryInterface(REFIID iid, void** punknown);
	STDMETHODIMP_(ULONG) AddRef() { return ++m_cRef;  }
	STDMETHODIMP_(ULONG) Release() { 
		auto local_ref{ --m_cRef };
		if (local_ref < 1)
			delete this;
		return local_ref;
	}

	STDMETHODIMP DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
	STDMETHODIMP DragLeave(void);
	STDMETHODIMP Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

private:

	HRESULT InspectIDataObject(IDataObject* pDataObject);

	ULONG m_cRef = 0;

	HWND m_hwndMainWindow = NULL;

	CComPtr<IDataObject> m_pCurrentDragDataObject = nullptr;
	std::wstring m_dragdrop_filepath;

	UINT m_cf_filename;
};