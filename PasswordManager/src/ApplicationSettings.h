#pragma once

#include <Windows.h>

struct ApplicationSettings {

	UINT		width	= 1200;
	UINT		height	= 800;
	UINT		x		= 100;
	UINT		y		= 100;

	ApplicationSettings();

	~ApplicationSettings();

};