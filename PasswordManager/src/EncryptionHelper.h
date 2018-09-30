#pragma once

bool DecryptToTempFile(const WCHAR * szFilename, const WCHAR * szPassword, WCHAR ** pszDecryptedFilename, BOOL * p_fHashMismatch);

bool EncryptFileAndSave(const WCHAR * szFilename, const WCHAR * szPassword, const WCHAR * szEncryptedFilename);

std::wstring GenerateRandomString(int cch);