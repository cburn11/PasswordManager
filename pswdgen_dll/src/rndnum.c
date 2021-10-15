#include <Windows.h>
#include <bcrypt.h>

#pragma comment(lib, "Bcrypt.lib")

BOOL GenerateRandomNumber(DWORD cbRequested, BYTE ** pOut, DWORD * pcbOut) {

	BCRYPT_HANDLE	hAlg = NULL;
	NTSTATUS		ret;

	BYTE			* pBuffer = NULL;

	ret = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RNG_ALGORITHM, NULL, 0);
	if( !BCRYPT_SUCCESS(ret) )		goto CLEANUP;

	pBuffer = (BYTE *) calloc(cbRequested, sizeof(BYTE));
	if( !BCRYPT_SUCCESS(ret) )		goto CLEANUP;

	ret = BCryptGenRandom(hAlg, (UCHAR *) pBuffer, cbRequested, 0);
	if( !BCRYPT_SUCCESS(ret) )		goto CLEANUP;

	*pcbOut = cbRequested;
	*pOut = pBuffer;

CLEANUP:
	if( hAlg )		BCryptCloseAlgorithmProvider(hAlg, 0);

	if( !BCRYPT_SUCCESS(ret) ) {
		*pcbOut = 0;
		*pOut = NULL;

		if( pBuffer )	free(pBuffer);

		return FALSE;
	}

	return TRUE;
}

int rndint(unsigned int min, unsigned int max) {

	double		quotient;
	int			rnd;
	DWORD		cbRnd;

	unsigned int	* ptemp;

	if( GenerateRandomNumber(sizeof(int), (BYTE **) &ptemp, &cbRnd) ) {

		quotient = *ptemp * 1.0 / UINT_MAX;

		rnd = min + quotient * (max + 1 - min);

		if( rnd == max + 1 )
			return max;

		free(ptemp);

		return rnd;

	}

	return -1;
}