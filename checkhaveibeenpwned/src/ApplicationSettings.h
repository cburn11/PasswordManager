#pragma once

#include <Windows.h>

#include <string>

class ApplicationSettings {

public:

	UINT		width	= 312;
	UINT		height	= 175;
	UINT		x		= 100;
	UINT		y		= 100;

	ApplicationSettings(std::wstring&& AppKey);

	~ApplicationSettings();

private:

	std::wstring m_AppKey;

};