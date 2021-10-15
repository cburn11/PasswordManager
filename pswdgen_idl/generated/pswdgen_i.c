

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Mon Jan 18 19:14:07 2038
 */
/* Compiler settings for src\pswdgen.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        EXTERN_C __declspec(selectany) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif // !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_PasswordGenerator,0x2B583A7F,0x51ED,0x4C17,0xA0,0xE6,0x5C,0x9A,0x44,0xC0,0xDA,0xA6);


MIDL_DEFINE_GUID(IID, IID_IPasswordGenerator,0x7A9E1877,0x913A,0x4E62,0xA5,0xD5,0x56,0x06,0x54,0xB8,0xCA,0x5F);


MIDL_DEFINE_GUID(IID, IID_IApplication,0x30363361,0xA497,0x4D49,0x9B,0xC4,0x75,0x9A,0xB1,0x5D,0x08,0xB2);


MIDL_DEFINE_GUID(CLSID, CLSID_PasswordGenerator,0x034042E4,0x0516,0x4AE3,0x84,0x6D,0x8B,0xEB,0x3C,0x17,0x98,0xAF);


MIDL_DEFINE_GUID(CLSID, CLSID_Application,0xF8F010D6,0x62DD,0x405C,0xAF,0x04,0x42,0xDD,0x34,0x77,0x60,0xAB);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



