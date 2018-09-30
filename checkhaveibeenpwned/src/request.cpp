#include "request.h"

web::json::value CheckAccount(const std::wstring& account) {

	if( account.length() < 1 )
		return web::json::value::string(U(""));

	web::http::client::http_client client{ U("https://haveibeenpwned.com/") };

	web::http::uri_builder ub{ U("api/v2/breachedaccount/") };
	ub.append(account);

	web::http::http_request request{ web::http::methods::GET };
	request.set_request_uri(ub.to_uri());

	web::http::http_response response = client.request(request).get();
	if( response.status_code() != web::http::status_codes::OK )
		return web::json::value::string(U(""));

	return response.extract_json(true).get();
}

std::wstring ParseJSONResponse(const web::json::value& json) {

	if( json.is_string() ) {

		return L"0 results.";
	}
	
	auto cResults = json.as_array().size();

	std::wstring results{ std::to_wstring(cResults) + L" result" + (cResults > 1 ? L"s" : L"") + L":\r\n\r\n" };

	for( auto breach : json.as_array() ) {

		results += breach[L"Name"].as_string() + L", " + breach[L"Domain"].as_string() + L": ";
		results += breach[L"BreachDate"].as_string() + L"\r\n";

		results += L"Compromised data: ";
		
		auto data_classes = breach[L"DataClasses"].as_array();
		auto cClasses = data_classes.size();
		for( auto data_class : data_classes ) {

			results += data_class.as_string() + (--cClasses > 0 ? L", " : L"");
		}

	}

	return results;
}