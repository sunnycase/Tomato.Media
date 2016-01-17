

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0613 */
/* at Tue Jan 19 11:14:07 2038
 */
/* Compiler settings for C:\Users\SUNNYC~1\AppData\Local\Temp\Tomato.Media.Effect.idl-2b53e298:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.00.0613 
    protocol : dce , ms_ext, c_ext, robust
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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __Tomato2EMedia2EEffect_h__
#define __Tomato2EMedia2EEffect_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(__cplusplus)
#if defined(__MIDL_USE_C_ENUM)
#define MIDL_ENUM enum
#else
#define MIDL_ENUM enum class
#endif
#endif


/* Forward Declarations */ 

#ifndef ____x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_FWD_DEFINED__
#define ____x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_FWD_DEFINED__
typedef interface __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps;

#ifdef __cplusplus
namespace ABI {
    namespace Tomato {
        namespace Media {
            namespace Effect {
                interface IEqualizerEffectProps;
            } /* end namespace */
        } /* end namespace */
    } /* end namespace */
} /* end namespace */

#endif /* __cplusplus */

#endif 	/* ____x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "inspectable.h"
#include "Windows.Media.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_Tomato2EMedia2EEffect_0000_0000 */
/* [local] */ 

#pragma warning(push)
#pragma warning(disable:4668) 
#pragma warning(disable:4001) 
#pragma once
#pragma warning(pop)
#pragma once
#pragma once
#if !defined(____x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_INTERFACE_DEFINED__)
extern const __declspec(selectany) _Null_terminated_ WCHAR InterfaceName_Tomato_Media_Effect_IEqualizerEffectProps[] = L"Tomato.Media.Effect.IEqualizerEffectProps";
#endif /* !defined(____x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_INTERFACE_DEFINED__) */


/* interface __MIDL_itf_Tomato2EMedia2EEffect_0000_0000 */
/* [local] */ 



extern RPC_IF_HANDLE __MIDL_itf_Tomato2EMedia2EEffect_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_Tomato2EMedia2EEffect_0000_0000_v0_0_s_ifspec;

#ifndef ____x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_INTERFACE_DEFINED__
#define ____x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_INTERFACE_DEFINED__

/* interface __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps */
/* [uuid][object] */ 



/* interface ABI::Tomato::Media::Effect::IEqualizerEffectProps */
/* [uuid][object] */ 


EXTERN_C const IID IID___x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps;

#if defined(__cplusplus) && !defined(CINTERFACE)
    } /* end extern "C" */
    namespace ABI {
        namespace Tomato {
            namespace Media {
                namespace Effect {
                    
                    MIDL_INTERFACE("DFC6F70B-BD21-47B9-ACA1-951F1C0E1B42")
                    IEqualizerEffectProps : public IInspectable
                    {
                    public:
                        virtual HRESULT STDMETHODCALLTYPE AddOrUpdateFilter( 
                            /* [in] */ FLOAT frequency,
                            /* [in] */ FLOAT bandWidth,
                            /* [in] */ FLOAT gain) = 0;
                        
                        virtual HRESULT STDMETHODCALLTYPE RemoveFilter( 
                            /* [in] */ FLOAT frequency) = 0;
                        
                    };

                    extern const __declspec(selectany) IID & IID_IEqualizerEffectProps = __uuidof(IEqualizerEffectProps);

                    
                }  /* end namespace */
            }  /* end namespace */
        }  /* end namespace */
    }  /* end namespace */
    extern "C" { 
    
#else 	/* C style interface */

    typedef struct __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectPropsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIids )( 
            __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps * This,
            /* [out] */ ULONG *iidCount,
            /* [size_is][size_is][out] */ IID **iids);
        
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeClassName )( 
            __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps * This,
            /* [out] */ HSTRING *className);
        
        HRESULT ( STDMETHODCALLTYPE *GetTrustLevel )( 
            __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps * This,
            /* [out] */ TrustLevel *trustLevel);
        
        HRESULT ( STDMETHODCALLTYPE *AddOrUpdateFilter )( 
            __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps * This,
            /* [in] */ FLOAT frequency,
            /* [in] */ FLOAT bandWidth,
            /* [in] */ FLOAT gain);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveFilter )( 
            __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps * This,
            /* [in] */ FLOAT frequency);
        
        END_INTERFACE
    } __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectPropsVtbl;

    interface __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps
    {
        CONST_VTBL struct __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectPropsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_GetIids(This,iidCount,iids)	\
    ( (This)->lpVtbl -> GetIids(This,iidCount,iids) ) 

#define __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_GetRuntimeClassName(This,className)	\
    ( (This)->lpVtbl -> GetRuntimeClassName(This,className) ) 

#define __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_GetTrustLevel(This,trustLevel)	\
    ( (This)->lpVtbl -> GetTrustLevel(This,trustLevel) ) 


#define __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_AddOrUpdateFilter(This,frequency,bandWidth,gain)	\
    ( (This)->lpVtbl -> AddOrUpdateFilter(This,frequency,bandWidth,gain) ) 

#define __x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_RemoveFilter(This,frequency)	\
    ( (This)->lpVtbl -> RemoveFilter(This,frequency) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* ____x_ABI_CTomato_CMedia_CEffect_CIEqualizerEffectProps_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_Tomato2EMedia2EEffect_0000_0001 */
/* [local] */ 

#ifdef __cplusplus
namespace ABI {
namespace Tomato {
namespace Media {
namespace Effect {
class EqualizerEffectTransform;
} /*Effect*/
} /*Media*/
} /*Tomato*/
}
#endif

#ifndef RUNTIMECLASS_Tomato_Media_Effect_EqualizerEffectTransform_DEFINED
#define RUNTIMECLASS_Tomato_Media_Effect_EqualizerEffectTransform_DEFINED
extern const __declspec(selectany) _Null_terminated_ WCHAR RuntimeClass_Tomato_Media_Effect_EqualizerEffectTransform[] = L"Tomato.Media.Effect.EqualizerEffectTransform";
#endif


/* interface __MIDL_itf_Tomato2EMedia2EEffect_0000_0001 */
/* [local] */ 




extern RPC_IF_HANDLE __MIDL_itf_Tomato2EMedia2EEffect_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_Tomato2EMedia2EEffect_0000_0001_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


