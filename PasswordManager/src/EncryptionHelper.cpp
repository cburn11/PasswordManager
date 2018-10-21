#include <Windows.h>

#include <bcrypt.h>

#include <stdlib.h>
#include <string.h>

#include <memory>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#pragma comment(lib, "bcrypt.lib")

BOOL hash(WCHAR * szAlg, BYTE * pIn, DWORD cbIn, BYTE ** ppHash, DWORD * pcbHash) {

	BCRYPT_ALG_HANDLE		hAlg = NULL;
	BCRYPT_HASH_HANDLE		hHash = NULL;

	NTSTATUS				ret;

	ULONG					cbHashObject, cbHash, cbWritten;

	BYTE					* pHashObject = NULL;
	BYTE					* pHash = NULL;

	ret = BCryptOpenAlgorithmProvider(&hAlg, szAlg, NULL, 0);
	if( !BCRYPT_SUCCESS(ret) )
		return FALSE;

	ret = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH,
		(UCHAR *) &cbHashObject, sizeof(cbHashObject), &cbWritten, 0);
	if( !BCRYPT_SUCCESS(ret) )
		goto CLEANUP;

	pHashObject = (BYTE *) calloc(cbHashObject, sizeof(BYTE));
	if( !pHashObject )
		goto CLEANUP;

	ret = BCryptCreateHash(hAlg, &hHash, pHashObject,
		cbHashObject, NULL, 0, 0);
	if( !BCRYPT_SUCCESS(ret) )
		goto CLEANUP;

	ret = BCryptHashData(hHash, (UCHAR *) pIn, cbIn, 0);
	if( !BCRYPT_SUCCESS(ret) )
		goto CLEANUP;

	ret = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH,
		(UCHAR *) &cbHash, sizeof(cbHash), &cbWritten, 0);
	if( !BCRYPT_SUCCESS(ret) )
		goto CLEANUP;

	pHash = (BYTE *) calloc(cbHash, sizeof(BYTE));
	if( !pHash )
		goto CLEANUP;

	ret = BCryptFinishHash(hHash, pHash, cbHash, 0);
	if( !BCRYPT_SUCCESS(ret) )
		goto CLEANUP;

	*ppHash = pHash;
	*pcbHash = cbHash;

CLEANUP:

	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	if( pHashObject )						free(pHashObject);
	if( pHash && !BCRYPT_SUCCESS(ret) )		free(pHash);

	if( BCRYPT_SUCCESS(ret) )
		return TRUE;
	else
		return FALSE;
}

BOOL LoadFile(const WCHAR * szFile, BYTE ** ppBuffer, DWORD * pcbBuffer) {

	DWORD	cbIn, cbFileSizeHigh, cbFileSizeLo;
	HANDLE	hIn;
	BYTE	* pIn;

	hIn = CreateFile(szFile, GENERIC_READ, 0, NULL, /*OPEN_ALWAYS*/OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if( INVALID_HANDLE_VALUE == hIn || NULL == hIn ) {
		DWORD error = GetLastError();
		if( error == ERROR_FILE_NOT_FOUND )	throw std::exception{ "FILE_NOT_FOUND" };
		return FALSE;
	}

	cbFileSizeLo = GetFileSize(hIn, &cbFileSizeHigh);

	pIn = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbFileSizeLo);

	ReadFile(hIn, pIn, cbFileSizeLo, &cbIn, NULL);

	*pcbBuffer = cbIn;

	*ppBuffer = pIn;

	CloseHandle(hIn);

	return TRUE;
}

BOOL Decrypt(const WCHAR * szSecret, BYTE * pCypher, DWORD cbCypher, BYTE ** ppPlain, DWORD * pcbPlain) {

	BCRYPT_HANDLE		hAlg;
	NTSTATUS			ret;

	BCRYPT_HANDLE		hKey;
	//DWORD				cbKey;
	//BYTE				* pKey;

	DWORD				cbBlockLength;

	DWORD				cbIV;
	BYTE				* pIV;

	DWORD				cbSecret, cbSecretHash;
	PBYTE				pSecretHash;

	DWORD				cbPlain;
	BYTE				* pPlain;

	DWORD				cbWritten;

	cbSecret = ( wcslen(szSecret) + 1 ) * sizeof(WCHAR);
	hash(BCRYPT_SHA256_ALGORITHM, (BYTE *) szSecret, cbSecret,
		&pSecretHash, &cbSecretHash);

	//	Obtain algorithm handle
	ret = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);

	//	Setup symmetric key
	/*ret = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH,
	(UCHAR *) &cbKey, sizeof(cbKey), &cbWritten, 0);
	pKey = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbKey);*/
	ret = BCryptGenerateSymmetricKey(hAlg, &hKey,
		/*(UCHAR *) pKey*/NULL, /*cbKey*/0, pSecretHash, cbSecretHash, 0);

	//	Setup IV
	ret = BCryptGetProperty(hAlg, BCRYPT_BLOCK_LENGTH,
		(UCHAR *) &cbBlockLength, sizeof(cbBlockLength), &cbWritten, 0);
	pIV = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBlockLength);
	cbIV = cbBlockLength;
	memcpy(pIV, pCypher, cbBlockLength);

	//	Setup plain text buffer
	ret = BCryptDecrypt(hKey, pCypher + cbBlockLength, cbCypher - cbBlockLength,
		NULL, pIV, cbIV, NULL, 0, &cbPlain, BCRYPT_BLOCK_PADDING);
	pPlain = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbPlain);
	ret = BCryptDecrypt(hKey, pCypher + cbBlockLength, cbCypher - cbBlockLength,
		NULL, pIV, cbIV, pPlain, cbPlain, &cbWritten, BCRYPT_BLOCK_PADDING);

	*pcbPlain = cbWritten;
	*ppPlain = pPlain;

	ret = BCryptDestroyKey(hKey);
	ret = BCryptCloseAlgorithmProvider(hAlg, 0);

	//HeapFree(GetProcessHeap(), 0, pKey);
	HeapFree(GetProcessHeap(), 0, pIV);

	free(pSecretHash);

	return TRUE;
}

BOOL SaveFile(WCHAR * szFile, BYTE * pBuffer, DWORD cbBuffer) {

	HANDLE		hOut;
	DWORD		cbWritten;

	hOut = CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(hOut, pBuffer, cbBuffer, &cbWritten, NULL);

	CloseHandle(hOut);

	return TRUE;
}

extern WCHAR * g_szTempFilePath;

bool DecryptToTempFile(const WCHAR * szFilename, const WCHAR * szPassword, WCHAR ** pszDecryptedFilename, BOOL * p_fHashMismatch) {

	bool ret = false;
	bool fHashMismatch = false;

	DWORD		cbFile;
	BYTE		* pFile = nullptr;

	DWORD		cbPlain;
	BYTE		* pPlain = nullptr;

	DWORD		cbHash;
	BYTE *		pHash = nullptr;

	if( LoadFile(szFilename, &pFile, &cbFile) ) {

		if( Decrypt(szPassword, pFile + 32, cbFile - 32, &pPlain, &cbPlain) ) {

			if( hash(BCRYPT_SHA256_ALGORITHM, pPlain, cbPlain, &pHash, &cbHash) ) {

				if( memcmp(pFile, pHash, cbHash) == 0 ) {

					WCHAR * szTempfilename = ( WCHAR * ) new WCHAR[MAX_PATH]{ 0 };
					if( GetTempFileName(g_szTempFilePath, L"PWM", 0, szTempfilename) ) {

						if( SaveFile(szTempfilename, pPlain, cbPlain) ) {
							*pszDecryptedFilename = szTempfilename;
							ret = true;
						}
					}

				} else {
					// Decrypted file's hash does not match the encrypted file's plaintext's hash.

					*p_fHashMismatch = TRUE;
				}

				free(pHash);
			}			
			
			HeapFree(GetProcessHeap(), 0, pPlain);
		}

		HeapFree(GetProcessHeap(), 0, pFile);
	}

	return ret;
}

BOOL Encrypt(const WCHAR * szSecret, BYTE * pPlain, DWORD cbPlain, BYTE ** ppCypher, DWORD * pcbCypher) {

	BCRYPT_HANDLE		hAlg;
	NTSTATUS			ret;

	BCRYPT_HANDLE		hKey;
	DWORD				cbKey;
	BYTE				* pKey;

	DWORD				cbIV;
	BYTE				* pIV;

	DWORD				cbSecret, cbSecretHash;
	PBYTE				pSecretHash;

	DWORD				cbCypher;
	BYTE				* pCypher;

	DWORD				cbWritten;

	cbSecret = ( wcslen(szSecret) + 1 ) * sizeof(WCHAR);
	hash(BCRYPT_SHA256_ALGORITHM, (BYTE *) szSecret, cbSecret,
		&pSecretHash, &cbSecretHash);

	//	Obtain algorithm handle
	ret = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);

	//	Setup symmetric key
	ret = BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, pSecretHash, cbSecretHash, 0);

	//	Setup IV
	ret = BCryptGetProperty(hAlg, BCRYPT_BLOCK_LENGTH,
		(UCHAR *) &cbIV, sizeof(cbIV), &cbWritten, 0);
	pIV = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbIV);
	ret = BCryptGenRandom(NULL, pIV, cbIV, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

	//	Enrypt plain text
	//		1. Determine size required for cypher text buffer
	ret = BCryptEncrypt(hKey, pPlain, cbPlain, NULL,
		NULL, 0, NULL, 0, &cbCypher, BCRYPT_BLOCK_PADDING);

	//		2. Setup cypher text buffer
	cbCypher += cbIV;
	pCypher = (BYTE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbCypher);
	memcpy(pCypher, pIV, cbIV);

	//		3. Encrpyt
	ret = BCryptEncrypt(hKey, pPlain, cbPlain, NULL, pIV, cbIV,
		pCypher + cbIV, cbCypher - cbIV, &cbWritten, BCRYPT_BLOCK_PADDING);

	cbWritten += cbIV;

	*pcbCypher = cbWritten;
	*ppCypher = pCypher;

	ret = BCryptDestroyKey(hKey);
	ret = BCryptCloseAlgorithmProvider(hAlg, 0);

	HeapFree(GetProcessHeap(), 0, pIV);

	free(pSecretHash);

	return TRUE;
}

BOOL SaveEncryptedFile(const WCHAR * szFile, BYTE * pBuffer, DWORD cbBuffer, BYTE * pHash, DWORD cbHash) {
	HANDLE		hOut;
	DWORD		cbWritten;

	hOut = CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(hOut, pHash, cbHash, &cbWritten, NULL);

	WriteFile(hOut, pBuffer, cbBuffer, &cbWritten, NULL);

	CloseHandle(hOut);

	return TRUE;
}

bool EncryptFileAndSave(const WCHAR * szFilename, const WCHAR * szPassword, const WCHAR * szEncryptedFilename) {

	DWORD		cbFile;
	BYTE		* pFile;

	DWORD		cbHash;
	BYTE *		pHash;

	DWORD		cbCypher;
	BYTE		* pCypher;

	bool		ret = false;

	if( LoadFile(szFilename, &pFile, &cbFile) ) {

		if( hash(BCRYPT_SHA256_ALGORITHM, (BYTE *) pFile, cbFile, &pHash, &cbHash) ) {

			if( Encrypt(szPassword, pFile, cbFile, &pCypher, &cbCypher) ) {

				if( SaveEncryptedFile(szEncryptedFilename, pCypher, cbCypher, pHash, cbHash) ) {
					ret = true;
				}

				HeapFree(GetProcessHeap(), 0, pCypher);
			}

			free(pHash);
		}

		HeapFree(GetProcessHeap(), 0, pFile);
	}

	return ret;
}

std::wstring GenerateRandomString(int cch) {

	int cbBuffer = cch / 2;
	if( cbBuffer < 1 )
		return L"";

	WCHAR hexBuffer[3]{ 0 };

	std::unique_ptr<unsigned char> buffer{ new unsigned char[cbBuffer] {0} };
	if( buffer ) {

		BCryptGenRandom(NULL, buffer.get(), cbBuffer, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

		std::wstringstream str_stream;
		for( int i = 0; i < cbBuffer; ++i ) {
			wsprintf(hexBuffer, L"%.2x", *(buffer.get() + i));
			str_stream << hexBuffer;
		}

		return std::wstring{ str_stream.str() };
	}

	return L"";
}