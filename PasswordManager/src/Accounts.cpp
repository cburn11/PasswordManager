#include <Windows.h>
#include <windowsx.h>

#include <atlcomcli.h>
#include <atlenc.h>

#include <MsXml6.h>
#pragma comment(lib, "msxml6.lib")

#include <string>
#include <vector>
#include <algorithm>

#include "Accounts.h"
#include "PasswordDialog.h"
#include "resource.h"
#include "EncryptionHelper.h"

using std::wstring;
using std::vector;
using std::begin;
using std::end;
using std::for_each;
using std::transform;

bool Accounts::PromptToSaveChanges() {

	auto ret = MessageBox(NULL, L"Save changes before closing?", L"Unsaved changes", MB_ICONQUESTION | MB_YESNO);
	return ( ret == IDYES ) ? true : false;

}

wstring SanitizeXML(const wstring& xml) {

	WCHAR * szEscapedXml = ( WCHAR * ) new WCHAR[xml.size() * 4]{ 0 };
	if( szEscapedXml && xml != L"" ) {
		EscapeXML(xml.c_str(), xml.size(), szEscapedXml, xml.size() * 4 - 1);

		wstring EscapedXml{ szEscapedXml };
		delete[] szEscapedXml;

		return EscapedXml;
	}

	return L"";
}

#define MEMBER_TO_XML(tag, value) L"<" #tag L">" + SanitizeXML(value) + L"</" #tag L">"

std::wstring Account::to_xml() const {

	std::wstring xml{ L"<account>" };
	std::wstring field;

	field = m_fields.at(Account::Field::ID);
	xml += MEMBER_TO_XML(id, field);

	field = m_fields.at(Account::Field::NAME);
	xml += MEMBER_TO_XML(name, field);

	field = m_fields.at(Account::Field::URL);
	xml += MEMBER_TO_XML(url, field);

	field = m_fields.at(Account::Field::USERNAME);
	xml += MEMBER_TO_XML(username, field);

	field = m_fields.at(Account::Field::PASSWORD);
	xml += MEMBER_TO_XML(password, field);

	field = m_fields.at(Account::Field::DESCRIPTION);
	xml += MEMBER_TO_XML(description, field);

	field = m_fields.at(Account::Field::USERNAMEFIELD);
	xml += MEMBER_TO_XML(usernamefield, field);

	field = m_fields.at(Account::Field::PASSWORDFIELD);
	xml += MEMBER_TO_XML(passwordfield, field);

	xml += L"</account>";	

	return xml;
}

Account::Account(IXMLDOMNode * pAccount) {
	HRESULT hr;

	CComPtr<IXMLDOMNodeList> pChildren;

	hr = pAccount->get_childNodes(&pChildren);

	if( SUCCEEDED(hr) && pChildren ) {
		long cChildren;
		hr = pChildren->get_length(&cChildren);

		for( long i = 0; i < cChildren; ++i ) {

			CComPtr<IXMLDOMNode> pNode;
			hr = pChildren->get_item(i, &pNode);

			if( pNode ) {

				BSTR bstrName, bstrText;
				pNode->get_nodeName(&bstrName);
				pNode->get_text(&bstrText);				

				auto field = getFieldValFromString(bstrName);
				if( field != Account::Field::NONE )
					m_fields[field] = bstrText;

				SysFreeString(bstrName);
				SysFreeString(bstrText);
			}
		}
	}
}

std::wstring& Account::operator[](Account::Field field) {

	return m_fields[field];

}

std::wstring& Account::operator[](const std::wstring& str) {

	auto field = getFieldValFromString(str);

	return m_fields[field];
}

Account::Field Account::getFieldValFromString(const std::wstring& str) const {

	try {

		return m_str_to_field.at(str);

	} catch( std::exception e ) {

		return Account::Field::NONE;

	}

}

std::vector<std::wstring> * Account::getStrings(DWORD fields) const {

	auto pStrs = new std::vector<std::wstring>{};

	if( fields & Account::Field::URL )	pStrs->push_back(m_fields.at(Account::Field::URL));
	if( fields & Account::Field::USERNAME )	pStrs->push_back(m_fields.at(Account::Field::USERNAME));
	if( fields & Account::Field::PASSWORD )	pStrs->push_back(m_fields.at(Account::Field::PASSWORD));
	if( fields & Account::Field::ID )	pStrs->push_back(m_fields.at(Account::Field::ID));
	if( fields & Account::Field::DESCRIPTION )	pStrs->push_back(m_fields.at(Account::Field::DESCRIPTION));
	if( fields & Account::Field::USERNAMEFIELD )	pStrs->push_back(m_fields.at(Account::Field::USERNAMEFIELD));
	if( fields & Account::Field::PASSWORDFIELD )	pStrs->push_back(m_fields.at(Account::Field::PASSWORDFIELD));

	return pStrs;
}

Account Account::clone() const {

	Account new_account{};

	for( const auto& [field, str] : m_fields ) {
		
		new_account.setString(field, str);

	}

	return new_account;
}

Accounts::Accounts() {

	m_hPersistentFile = INVALID_HANDLE_VALUE;

	HRESULT hr = OleInitialize(nullptr);

#ifdef	_DEBUG
	OutputDebugString(L"Accounts Construction\n");
#endif	//_DEBUG

}

Accounts::~Accounts() {

	if( m_fUnsavedChanges ) {
		if( PromptToSaveChanges() )
			SaveAccounts();
	}

	if( m_hPersistentFile )
		CloseHandle(m_hPersistentFile);

	OleUninitialize();

#ifdef	_DEBUG
	OutputDebugString(L"Accounts Destruction\n");
#endif	//_DEBUG
}

bool Accounts::Load(const WCHAR * szFilename) {

	auto szExt = wcsrchr(szFilename, L'.') + 1;
	
	if( _wcsicmp(szExt, L"pwm") == 0 ) {

		return LoadFromEncrypted(szFilename);

	} else if( _wcsicmp(szExt, L"xml") == 0 ) {

		return LoadFromXml(szFilename);

	} else {

		//	For now, treat an unknown extension
		//	as xml.

		return LoadFromXml(szFilename);
	}

	return false;
}

extern HWND		g_hwndApp;

WCHAR * PromptForPassword() {
	
	HINSTANCE hInstance = (HINSTANCE) GetModuleHandle(NULL);

	return (WCHAR *) DialogBox(
		hInstance, MAKEINTRESOURCE(IDD_DIALOG_PASSWORD), 
		g_hwndApp, PasswordDialog::PasswordDlgProc);
}

bool Accounts::LoadFromEncrypted(const WCHAR * szFilename) {
		
	WCHAR * szPassword = PromptForPassword();
	
	if( !szPassword ) return false;

	const WCHAR * szDecryptedFilename = nullptr;
	BOOL fHashMismatch = false;
	bool fdecrypted = false;
	
	try {

		fdecrypted = DecryptToTempFile(szFilename, szPassword, &szDecryptedFilename, &fHashMismatch);

	} catch( std::exception ) {
		
		throw FILE_NOT_FOUND{  };
	}

	m_password = szPassword;
	delete[] szPassword;
	
	if( fdecrypted ) {

		m_fOriginalFileEncrypted = true;
		m_encrypted_filepath = szFilename;

		bool floaded = LoadFromXml(szDecryptedFilename);
		DeleteFile(szDecryptedFilename);
		delete[] szDecryptedFilename;
		return floaded;

	} else {

		if( fHashMismatch == TRUE ) {
			auto query = MessageBox(g_hwndApp, L"Enter password again?", L"Password incorrect", MB_ICONERROR | MB_YESNO);
			if( IDYES == query )
				return LoadFromEncrypted(szFilename);
		}
	}

	return false;
}

bool Accounts::LoadFromXml(const WCHAR * szFilename) {

	auto res = GetFileAttributesEx(szFilename, GetFileExInfoStandard, &m_filedata);
	if( 0 == res ) {
		auto error = GetLastError();
		if( ERROR_FILE_NOT_FOUND == error )	throw FILE_NOT_FOUND{};
		return false;
	}

	HRESULT		hr;
	CComPtr<IXMLDOMDocument> m_pDoc;
	hr = CoCreateInstance(CLSID_DOMDocument60, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&m_pDoc));
	if( !SUCCEEDED(hr) || !m_pDoc )
		return false;

	CComVariant	varPath{ SysAllocString(szFilename) };
	VARIANT_BOOL	result;

	hr = m_pDoc->load(varPath, &result);

	if( SUCCEEDED(hr) && result == VARIANT_TRUE ) {


		CComPtr<IXMLDOMElement> pRoot;
		hr = m_pDoc->get_documentElement(&pRoot);
		if( SUCCEEDED(hr) && pRoot ) {

			CComPtr<IXMLDOMNodeList> pAccounts;
			CComBSTR bstrTagName{ L"account" };
			hr = pRoot->getElementsByTagName(bstrTagName, &pAccounts);
			if( SUCCEEDED(hr) && pAccounts ) {
				long cAccounts;
				hr = pAccounts->get_length(&cAccounts);
				m_Accounts.reserve(cAccounts);
				for( long i = 0; i < cAccounts; ++i ) {
					CComPtr<IXMLDOMNode> pAccount;
					hr = pAccounts->get_item(i, &pAccount);
					if( SUCCEEDED(hr) && pAccount ) {
										
						Account account{ pAccount };
						account.index = i;
						m_Accounts.push_back(std::move(account));

					}
				}

				m_filepath = szFilename;

				return true;
			}
		}
	}

	return false;
}

enum GetFilenameType {
	open	= 1,
	save	= 2,
	saveEnc = 3
};

/*		Defined in App.cpp	*/
bool GetFilename(HWND hwndParent, GetFilenameType type, const WCHAR ** pszFilename);

bool Accounts::SaveAccounts() {

	if( !m_fUnsavedChanges )
		return true;

	if( IsBackedByFile() ) {

		if( m_fOriginalFileEncrypted ) {

			return SaveAccountsEncrypted(m_encrypted_filepath.c_str());

		} else {

			if( OutsideModificationAfterOpening() ) {
				
				wstring msg{ m_filepath };
				msg += L" has been modified by another program since it was opened by PasswordManager. Do you " \
					L"want to save to another file to avoid overwritting those outside changes?";
				auto ret = MessageBox(NULL, msg.c_str(), L"Overwrite outside changes", MB_ICONQUESTION | MB_YESNO);
				if( ret == IDYES )
					goto SAVEAS;
			}

			return Accounts::SaveAccountsAs(m_filepath.c_str());
		}

	}

SAVEAS:

	const WCHAR * szFilename;
	if( ::GetFilename(NULL, GetFilenameType::save, &szFilename) ) {
		return Accounts::SaveAccountsAs(szFilename);
		delete[] szFilename;

	}

	return false;
}

bool Accounts::SaveAccountsAs(const WCHAR * szFilename) {
	
	wstring xml{ L"<passwordmanager><accounts>" };

	auto AccountToXml = [&xml](const Account& account) {
		xml += account.to_xml();
	};
	for_each(std::begin(m_Accounts), std::end(m_Accounts), AccountToXml);

	xml += L"</accounts></passwordmanager>";

	CComPtr<IXMLDOMDocument> m_pDoc;
	HRESULT hr = CoCreateInstance(CLSID_DOMDocument60, nullptr, CLSCTX_INPROC, IID_PPV_ARGS(&m_pDoc));
	if( !SUCCEEDED(hr) || !m_pDoc )
		return false;

	VARIANT_BOOL res;
	hr = m_pDoc->loadXML(CComBSTR{ xml.c_str() }, &res);
	if( S_OK == hr ) {

		//CloseHandle(m_hPersistentFile);
		hr = m_pDoc->save(CComVariant{ SysAllocString(szFilename) });
		/*m_hPersistentFile = GetFileHandle(szFilename, GENERIC_READ | GENERIC_WRITE);
		if( m_hPersistentFile == INVALID_HANDLE_VALUE ) {

		DWORD error = GetLastError();*/
		if( S_OK != hr ) {

			wstring msg{ L"Error (" };
			msg += std::to_wstring(hr);
			msg += L") opening ";
			msg += szFilename;
			MessageBox(NULL, msg.c_str(), L"Error opening file.", MB_ICONERROR);

		} else {

			if( m_filepath != szFilename && m_fOriginalFileEncrypted) {

				m_fOriginalFileEncrypted = false;
				m_password = L"";
				m_encrypted_filepath = L"";

			}

			m_filepath = szFilename;
			GetFileAttributesEx(szFilename, GetFileExInfoStandard, &m_filedata);
			m_fUnsavedChanges = false;

			return true;
		}
	}

	return false;
}

extern WCHAR *	g_szTempFilePath;

bool Accounts::SaveAccountsEncrypted(const WCHAR * szFilename) {

	if( m_fOriginalFileEncrypted ) {

		SaveAccountsAs(m_filepath.c_str());

		if( m_encrypted_filepath != szFilename ) {
			m_encrypted_filepath = szFilename;
			m_password = PromptForPassword();
		}

		auto ret = EncryptFileAndSave(m_filepath.c_str(), m_password.c_str(), m_encrypted_filepath.c_str());

		if( ret ) DeleteFile(m_filepath.c_str());

		return ret;

	} else {

		WCHAR szTempfile[MAX_PATH]{ 0 };
		GetTempFileName(g_szTempFilePath, L"PWM", 0, szTempfile);
		m_filepath = szTempfile;
		GetFileAttributesEx(szTempfile, GetFileExInfoStandard, &m_filedata);

		SaveAccountsAs(m_filepath.c_str());

		WCHAR * szPassword = PromptForPassword();
		if( szPassword ) {

			bool ret = EncryptFileAndSave(m_filepath.c_str(), szPassword, szFilename);

			if( ret ) {

				m_password = szPassword;
				m_encrypted_filepath = szFilename;
				m_fOriginalFileEncrypted = true;

				DeleteFile(m_filepath.c_str());
			}

			delete[] szPassword;

			return ret;
		}
	}
	
	return true;
}

bool Accounts::OutsideModificationAfterOpening() {

	if( !IsBackedByFile() )
		return false;

	WIN32_FILE_ATTRIBUTE_DATA	filedata;

	if( !GetFileAttributesEx(m_filepath.c_str(), GetFileExInfoStandard, &filedata) ) {
		DWORD error = GetLastError();
		return false;
	}

	if( filedata.ftLastWriteTime.dwHighDateTime == m_filedata.ftLastWriteTime.dwHighDateTime &&
		filedata.ftLastWriteTime.dwLowDateTime == m_filedata.ftLastWriteTime.dwLowDateTime )

		return false;

	return true;
}

template const Account& Accounts::AddAccount(Account&&, int, bool);

template <typename T1>
const Account& Accounts::AddAccount(T1&& account, int index, bool fReindex) {

	//Account temp = account;

	//temp.index = m_Accounts.size();

	m_fUnsavedChanges = true;

	int inserted_index = 0;

	if( index < 0 ) {

		account.index = m_Accounts.size();
		inserted_index = account.index;
		m_Accounts.emplace_back(std::forward<T1>(account));

	} else {

		index = min(m_Accounts.size(), index);
		inserted_index = index;
		account.index = index;

		m_Accounts.emplace(m_Accounts.begin() + index, std::forward<T1>(account));

		if( fReindex )
			Reindex(index + 1);
	}

	return *( m_Accounts.begin() + inserted_index );
}

void Accounts::AddAccounts(Accounts&& accounts) {

	for( auto& account : accounts.m_Accounts ) {

		AddAccount(std::move(account));
	}
}

void Accounts::Reindex(int begin_index) {

	auto i = m_Accounts.begin() + begin_index;

	for_each(i, std::end(m_Accounts), [&begin_index](Account& account) {
		account.index = begin_index++;
	});
}

Account Accounts::RemoveAccount(long index, bool fReindex) {

	if( index >= m_Accounts.size() )
		return Account{};

	// Remove the account
	auto i = m_Accounts.begin() + index;
	Account retAccount{ std::move(*i) };
	m_Accounts.erase(i);

	m_fUnsavedChanges = true;
	
	/*if( index >= m_Accounts.size() )
		return Account{};*/

	// Update index values of the remaining accounts
	if( fReindex )
		Reindex(index);

	return std::move(retAccount);
}

const WCHAR * Accounts::GetFilename() const {

	if( m_filepath.size() < 1 )
		return nullptr;

	const WCHAR * lastslash = wcsrchr(m_filepath.c_str(), L'\\');
	if( lastslash != NULL )
		++lastslash;
	else
		lastslash = m_filepath.c_str();

	return lastslash;
}

const std::wstring& Accounts::GetFilepath() const { 

	if( m_fOriginalFileEncrypted )
		return m_encrypted_filepath;

	return m_filepath; 
}

int Accounts::SwapAccounts(int srcIndex, int dstIndex) {

	if( srcIndex < 0 || dstIndex < 0 )
		return -1;

	auto size = m_Accounts.size();

	dstIndex = max(0, min(size - 1, dstIndex));
	srcIndex = max(0, min(size - 1, srcIndex));

	if( srcIndex == dstIndex )
		return -1;

	auto& srcAccount = m_Accounts.at(srcIndex);
	auto& dstAccount = m_Accounts.at(dstIndex);

	std::swap(srcAccount, dstAccount);
	std::swap(srcAccount.index, dstAccount.index);

	m_fUnsavedChanges = true;

	return dstAccount.index;
}

int Accounts::MoveAccount(int srcIndex, int dstIndex) {

	auto account = RemoveAccount(srcIndex, false);

	const auto& moved_account = AddAccount(std::move(account), dstIndex, false);

	Reindex(min(srcIndex, dstIndex));

	return moved_account.index;
}

const std::vector<const Account *> * Accounts::Filter(const WCHAR * szFilter, Filter_Type type) {

	m_Filter.reset(new Account_Filter{ szFilter, type, &m_Accounts });

	return m_Filter->GetFilteredAccounts();
}

Accounts::Account_Filter::Account_Filter(const WCHAR * szFilter, Filter_Type type, const std::vector<Account> * pAccounts) {

	if( pAccounts->size() < 1 )
		return;

	wstring filter = { szFilter };
	if( filter.size() == 0 )
		return;

	transform(begin(filter), end(filter), begin(filter), tolower);

	Account::Field field;

	switch( type ) {

	case Filter_Type::Name:
		field = Account::Field::NAME;
		break;

	case Filter_Type::Url:
		field = Account::Field::URL;
		break;

	case Filter_Type::Username:
		field = Account::Field::USERNAME;
		break;

	case Filter_Type::Description:
		field = Account::Field::DESCRIPTION;
		break;
	}

	for( auto& account : *pAccounts ) {

		wstring target = account.getString(field);

		transform(begin(target), end(target), begin(target), tolower);

		auto srch = target.find(filter);
		if( srch != wstring::npos)
			m_FilteredAccounts.push_back(&account);
	}
}