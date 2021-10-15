

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __pswdgen_h_h__
#define __pswdgen_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPasswordGenerator_FWD_DEFINED__
#define __IPasswordGenerator_FWD_DEFINED__
typedef interface IPasswordGenerator IPasswordGenerator;

#endif 	/* __IPasswordGenerator_FWD_DEFINED__ */


#ifndef __IApplication_FWD_DEFINED__
#define __IApplication_FWD_DEFINED__
typedef interface IApplication IApplication;

#endif 	/* __IApplication_FWD_DEFINED__ */


#ifndef __PasswordGenerator_FWD_DEFINED__
#define __PasswordGenerator_FWD_DEFINED__

#ifdef __cplusplus
typedef class PasswordGenerator PasswordGenerator;
#else
typedef struct PasswordGenerator PasswordGenerator;
#endif /* __cplusplus */

#endif 	/* __PasswordGenerator_FWD_DEFINED__ */


#ifndef __Application_FWD_DEFINED__
#define __Application_FWD_DEFINED__

#ifdef __cplusplus
typedef class Application Application;
#else
typedef struct Application Application;
#endif /* __cplusplus */

#endif 	/* __Application_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __PasswordGenerator_LIBRARY_DEFINED__
#define __PasswordGenerator_LIBRARY_DEFINED__

/* library PasswordGenerator */
/* [version][uuid] */ 




EXTERN_C const IID LIBID_PasswordGenerator;

#ifndef __IPasswordGenerator_INTERFACE_DEFINED__
#define __IPasswordGenerator_INTERFACE_DEFINED__

/* interface IPasswordGenerator */
/* [object][oleautomation][nonextensible][dual][uuid] */ 


EXTERN_C const IID IID_IPasswordGenerator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7A9E1877-913A-4E62-A5D5-560654B8CA5F")
    IPasswordGenerator : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [in] */ BSTR propertyname,
            /* [retval][out] */ BSTR *propertyvalue) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetProperty( 
            /* [in] */ BSTR propertyname,
            /* [in] */ BSTR propertyvalue,
            /* [retval][out] */ VARIANT_BOOL *pres) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GeneratePassword( 
            /* [retval][out] */ BSTR *password) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IPasswordGeneratorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPasswordGenerator * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPasswordGenerator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPasswordGenerator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPasswordGenerator * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPasswordGenerator * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPasswordGenerator * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPasswordGenerator * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IPasswordGenerator * This,
            /* [in] */ BSTR propertyname,
            /* [retval][out] */ BSTR *propertyvalue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IPasswordGenerator * This,
            /* [in] */ BSTR propertyname,
            /* [in] */ BSTR propertyvalue,
            /* [retval][out] */ VARIANT_BOOL *pres);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GeneratePassword )( 
            IPasswordGenerator * This,
            /* [retval][out] */ BSTR *password);
        
        END_INTERFACE
    } IPasswordGeneratorVtbl;

    interface IPasswordGenerator
    {
        CONST_VTBL struct IPasswordGeneratorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPasswordGenerator_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPasswordGenerator_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPasswordGenerator_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPasswordGenerator_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IPasswordGenerator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IPasswordGenerator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IPasswordGenerator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IPasswordGenerator_GetProperty(This,propertyname,propertyvalue)	\
    ( (This)->lpVtbl -> GetProperty(This,propertyname,propertyvalue) ) 

#define IPasswordGenerator_SetProperty(This,propertyname,propertyvalue,pres)	\
    ( (This)->lpVtbl -> SetProperty(This,propertyname,propertyvalue,pres) ) 

#define IPasswordGenerator_GeneratePassword(This,password)	\
    ( (This)->lpVtbl -> GeneratePassword(This,password) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPasswordGenerator_INTERFACE_DEFINED__ */


#ifndef __IApplication_INTERFACE_DEFINED__
#define __IApplication_INTERFACE_DEFINED__

/* interface IApplication */
/* [object][oleautomation][nonextensible][dual][uuid] */ 


EXTERN_C const IID IID_IApplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("30363361-A497-4D49-9BC4-759AB15D08B2")
    IApplication : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Visible( 
            /* [retval][out] */ VARIANT_BOOL *pvis) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Visible( 
            /* [in] */ VARIANT_BOOL vis) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Quit( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetProperty( 
            /* [in] */ BSTR propertyname,
            /* [retval][out] */ BSTR *propertyvalue) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SetProperty( 
            /* [in] */ BSTR propertyname,
            /* [in] */ BSTR propertyvalue,
            /* [retval][out] */ VARIANT_BOOL *pres) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GeneratePassword( 
            /* [retval][out] */ BSTR *password) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IApplication * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IApplication * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IApplication * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IApplication * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IApplication * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IApplication * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IApplication * This,
            /* [retval][out] */ VARIANT_BOOL *pvis);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IApplication * This,
            /* [in] */ VARIANT_BOOL vis);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Quit )( 
            IApplication * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IApplication * This,
            /* [in] */ BSTR propertyname,
            /* [retval][out] */ BSTR *propertyvalue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IApplication * This,
            /* [in] */ BSTR propertyname,
            /* [in] */ BSTR propertyvalue,
            /* [retval][out] */ VARIANT_BOOL *pres);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GeneratePassword )( 
            IApplication * This,
            /* [retval][out] */ BSTR *password);
        
        END_INTERFACE
    } IApplicationVtbl;

    interface IApplication
    {
        CONST_VTBL struct IApplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IApplication_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IApplication_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IApplication_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IApplication_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IApplication_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IApplication_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IApplication_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IApplication_get_Visible(This,pvis)	\
    ( (This)->lpVtbl -> get_Visible(This,pvis) ) 

#define IApplication_put_Visible(This,vis)	\
    ( (This)->lpVtbl -> put_Visible(This,vis) ) 

#define IApplication_Quit(This)	\
    ( (This)->lpVtbl -> Quit(This) ) 

#define IApplication_GetProperty(This,propertyname,propertyvalue)	\
    ( (This)->lpVtbl -> GetProperty(This,propertyname,propertyvalue) ) 

#define IApplication_SetProperty(This,propertyname,propertyvalue,pres)	\
    ( (This)->lpVtbl -> SetProperty(This,propertyname,propertyvalue,pres) ) 

#define IApplication_GeneratePassword(This,password)	\
    ( (This)->lpVtbl -> GeneratePassword(This,password) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IApplication_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_PasswordGenerator;

#ifdef __cplusplus

class DECLSPEC_UUID("034042E4-0516-4AE3-846D-8BEB3C1798AF")
PasswordGenerator;
#endif

EXTERN_C const CLSID CLSID_Application;

#ifdef __cplusplus

class DECLSPEC_UUID("F8F010D6-62DD-405C-AF04-42DD347760AB")
Application;
#endif
#endif /* __PasswordGenerator_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


