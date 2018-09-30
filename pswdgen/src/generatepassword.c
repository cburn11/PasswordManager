#include <Windows.h>

#include "rndnum.h"

#define LOWER_INDEX		0
#define UPPER_INDEX		1
#define DIGITS_INDEX	2
#define SYMBOLS_INDEX	3
#define SPACES_INDEX	4

BOOL GeneratePassword(int * pCounts, WCHAR ** pszPassword, int * pcchPassword) {

	int		i, cchPassword, type;
	WCHAR	* szPassword;

	cchPassword = 0;

	for( i = 0; i < 5; ++i ) {
		cchPassword += pCounts[i];
	}

	if( cchPassword > 0 )
		szPassword = (WCHAR *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (cchPassword + 1) * sizeof(WCHAR));
	else
		return FALSE;

	if( !szPassword )
		return FALSE;

	for( i = 0; i < cchPassword; ++i ) {

		WCHAR	c;

		if( i == cchPassword - pCounts[SPACES_INDEX] ) {

			type = 4;

		} else {
SELECT:
			if( i == 0 || i == (cchPassword - 1) )
				type = rndint(0, 3);
			else
				type = rndint(0, 4);

			if( pCounts[type] < 1 )
				goto SELECT;
		}

		UINT upper = 0, lower = 0;

		switch( type ) {

		case 0:	//	lower
			//c = (WCHAR) rndint(L'a', L'z');
			lower = (UINT) 'a';
			upper = (UINT) 'z';
			break;

		case 1:	//	upper
			//c = (WCHAR) rndint(L'A', L'Z');
			lower = (UINT) 'A';
			upper = (UINT) 'Z';
			break;

		case 2:	//	digits
			//c = (WCHAR) rndint(L'0', L'9');
			lower = (UINT) '0';
			upper = (UINT) '9';
			break;

		case 3:	//	symbols
			
			lower = 33;
			upper = 64;
			break;

		case 4:	//	spaces
			c = L' ';
			break;
		}

		int temp = rndint(lower, upper);
		if( type == 3 ) {
			if( temp > 47 )
				temp += 10;

			if( temp > 64 )
				temp += 26;

			if( temp > 96 )
				temp += 26;
		}
		if( type != 4 )
			c = (WCHAR) temp;
		
		pCounts[type] -= 1;

		*(szPassword + i) = c;
	}

	*pszPassword = szPassword;

	if( pcchPassword )
		*pcchPassword = cchPassword;

	return TRUE;
}