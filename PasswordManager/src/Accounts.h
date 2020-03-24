#pragma once

#include <Windows.h>

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <utility>

#include <string.h>

#include <atlcomcli.h>
#include <MsXml6.h>

#include "resource.h"

struct FILE_NOT_FOUND : public std::exception { };

using Filter_Type = enum {
	Name = IDC_RADIO_NAME,
	Url = IDC_RADIO_URL,
	Username = IDC_RADIO_USERNAME,
	Description = IDC_RADIO_DESCRIPTION
};

class Account {

public:

	enum Field {
		ID = 0x01,
		NAME = 0x02,
		URL = 0x04,
		USERNAME = 0x08,
		PASSWORD = 0x10,
		DESCRIPTION = 0x20,
		USERNAMEFIELD = 0x40,
		PASSWORDFIELD = 0x80,
		ALL = 0xFF,
		NONE = 0x00
	};

	using account_pair = std::pair<Account::Field, std::wstring>;

	unsigned int	index;

	Account() = default;

	Account(IXMLDOMNode * pnode);

	Account(const Account& cp_Account) = delete;
	Account(Account&& cp_Account) = default;
	
	Account& operator=(const Account& cp_Account) = delete;
	Account& operator=(Account&& cp_Account) = default;

	std::wstring to_string() const {
		auto& name = m_fields.at(Account::Field::NAME);
		auto& username = m_fields.at(Account::Field::USERNAME);
		return name + L" " + username;
	}

	std::wstring to_xml() const;

	operator std::wstring() {
		return this->to_string();
	}

	const std::vector<std::wstring> * getStrings(DWORD field) const;

	std::wstring getString(Account::Field field) const { return m_fields.at(field); };
	std::wstring getString(const std::wstring& str) const {
		auto field = getFieldValFromString(str);
		return getString(field);
	}

	void setString(Account::Field field, const std::wstring& str) { m_fields[field] = str; }
	void setString(const std::wstring& key, const std::wstring str) {
		auto field = getFieldValFromString(key);
		setString(field, str);
	}

	std::wstring& operator[](Account::Field field);
	std::wstring& operator[](const std::wstring& str);

	Account::Field getFieldValFromString(const std::wstring& str) const;

private:

	std::unordered_map<Account::Field, std::wstring> m_fields;

	std::map<std::wstring, Account::Field> m_str_to_field = {
		{L"id", Account::Field::ID},
		{L"name", Account::Field::NAME},
		{L"url", Account::Field::URL},
		{L"username", Account::Field::USERNAME},
		{L"password", Account::Field::PASSWORD},
		{L"description", Account::Field::DESCRIPTION},
		{L"usernamefield", Account::Field::USERNAMEFIELD},
		{L"passwordfield", Account::Field::PASSWORDFIELD}
	};

};

class Accounts {	

	class Account_Filter {

		std::vector<const Account *> m_FilteredAccounts;

	public:

		Account_Filter(const WCHAR * szFilter, Filter_Type type, const std::vector<Account> * pAccounts);

		inline const std::vector<const Account *> * GetFilteredAccounts() { return &m_FilteredAccounts; }
	};

	std::vector<Account>		m_Accounts;

	std::unique_ptr<Account_Filter>		m_Filter;

	std::wstring				m_filepath; 

	bool						m_fUnsavedChanges = false;

	bool						m_fOriginalFileEncrypted = false;
	std::wstring				m_encrypted_filepath;
	std::wstring				m_password;

	//	1.6.18: file handle member is not used anymore
	HANDLE						m_hPersistentFile{ INVALID_HANDLE_VALUE };

	WIN32_FILE_ATTRIBUTE_DATA	m_filedata;

	bool PromptToSaveChanges();

	inline HANDLE GetFileHandle(const WCHAR * szFilename, DWORD dwDesiredAccess) {
		return CreateFile(szFilename, dwDesiredAccess, FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	bool LoadFromXml(const WCHAR * szFilename);
	bool LoadFromXml(const std::wstring& Filename) { return LoadFromXml(Filename.c_str()); }

	bool LoadFromEncrypted(const WCHAR * szFilename);
	bool LoadFromEncrypted(const std::wstring& Filename) { return LoadFromEncrypted(Filename.c_str()); }

	void Reindex(int begin_index = 0);

public:

	Accounts();

	~Accounts();

	Accounts(const Accounts&) = delete;
	Accounts& operator=(const Accounts&) = delete;

	bool Load(const WCHAR * szFilename);
	bool Load(const std::wstring& filename) { return Load(filename.c_str()); }

	bool IsBackedByFile() { return ( m_filepath.size() > 0 ) ? true : false; }

	bool OutsideModificationAfterOpening();

	template <typename T1>
	const Account& AddAccount(T1&& account, int index = -1, bool fReindex = true);
	void AddAccounts(Accounts&& accounts);

	Account RemoveAccount(long index, bool fReindex = true);

	template <typename T>
	void ReplaceAccount(long index, T&& account) {
		Account& targetaccount = m_Accounts.at(index);
		account.index = index;
		targetaccount = std::forward<Account>(account);
		m_fUnsavedChanges = true;
	}

	bool SaveAccounts();

	bool SaveAccountsAs(const WCHAR * szFilename);
	bool SaveAccountsAs(const std::wstring& Filename) { return SaveAccountsAs(Filename.c_str()); }

	bool SaveAccountsEncrypted(const WCHAR * szFilename);
	bool SaveAccountsEncrypted(const std::wstring& Filename) { return SaveAccountsEncrypted(Filename.c_str()); }

	const std::vector<Account>& GetAccounts() const { return m_Accounts; }

	const std::wstring& GetFilepath() const;

	const WCHAR * GetFilename() const ;

	const std::vector<const Account *> * Filter(const WCHAR * szFilter, Filter_Type type);

	int SwapAccounts(int srcIndex, int dstIndex);

	int MoveAccount(int srcIndex, int dstIndex);
};