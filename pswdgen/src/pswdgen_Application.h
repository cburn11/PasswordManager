#pragma once

#include <Windows.h>

#define WM_APP_GENERATE_PASSWORD (WM_APP + 1)
#define WM_APP_SET_PROPERTY (WM_APP + 2)
#define WM_APP_GET_PROPERTY (WM_APP + 3)

#ifdef __cplusplus
extern "C" {
#endif

	void* CreateApplication(HWND hwnd, IPasswordGenerator * p_pwgen);

	void DeleteApplication(void* papp);

#ifdef __cplusplus
}
#endif