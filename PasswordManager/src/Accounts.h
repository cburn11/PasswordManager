#pragma once

#include <Windows.h>

#include <memory>
#include <vector>
#include <string>

#include <string.h>

#include <atlcomcli.h>
#include <MsXml6.h>

#include "resource.h"

using Filter_Type = enum {
	Name = IDC_RADIO_NAME,
	Url = IDC_RADIO_URL,
	Username = IDC_RADIO_USERNAME,
	Description = IDC_RADIO_DESCRIPTION
};

class Account {

public:

	unsigned int	index;

	std::wstring	id;
	std::wstring	name;
	std::wstring	url;
	std::wstring	username;
	std::wstring	password;
	std::wstring	description;
	std::wstring	usernamefield;
	std::wstring	passwordfield;

	Account() = default;

	Account(IXMLDOMNode * pnode);

	Account(const Account& cp_Account) = default;
	Account(Account&& cp_Account) = default;
	
	Account& operator=(const Account& cp_Account) = delete;
	Account& operator=(Account&& cp_Account) = default;

	std::wstring to_string() const {
		return name + L" " + username;
	}

	std::wstring to_xml() const;

	operator std::wstring() {
		return this->to_string();
	}
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