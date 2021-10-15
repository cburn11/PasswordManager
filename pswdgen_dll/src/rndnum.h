#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

	BOOL GenerateRandomNumber(DWORD cbRequested, BYTE** pOut, DWORD* pcbOut);

	int rndint(unsigned int min, unsigned int max);

#ifdef __cplusplus
}
#endif