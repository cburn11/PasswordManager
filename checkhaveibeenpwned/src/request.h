#pragma once

#include <Windows.h>

#include <string>

#include <cpprest\http_client.h>

#pragma comment(lib, "cpprest_2_10.lib")

web::json::value CheckAccount(const std::wstring& account);

std::wstring ParseJSONResponse(const web::json::value& json);