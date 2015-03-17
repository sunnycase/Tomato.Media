//
// Tomato Media
// Media Foundation ByteStream Handler 基类
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#pragma once
#include "../include/tomato.media.core.h"

NSDEF_TOMATO_MEDIA

class ByteStreamHandlerBase : public wrl::RuntimeClass<wrl::RuntimeClassFlags<
	wrl::RuntimeClassType::WinRtClassicComMix>,
	ABI::Windows::Media::IMediaExtension,
	IMFByteStreamHandler>
{
public:
	ByteStreamHandlerBase();
	virtual ~ByteStreamHandlerBase();

	// IMediaExtension
	IFACEMETHOD(SetProperties) (ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration);

	// IMFAsyncCallback
	STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue);

	// IMFByteStreamHandler
	//从字节流创建 APESource
	STDMETHODIMP BeginCreateObject(
		/* [in] */ IMFByteStream *pByteStream,
		/* [in] */ LPCWSTR pwszURL,
		/* [in] */ DWORD dwFlags,
		/* [in] */ IPropertyStore *pProps,
		/* [out] */ IUnknown **ppIUnknownCancelCookie,
		/* [in] */ IMFAsyncCallback *pCallback,
		/* [in] */ IUnknown *punkState);

	STDMETHODIMP EndCreateObject(
		/* [in] */ IMFAsyncResult *pResult,
		/* [out] */ MF_OBJECT_TYPE *pObjectType,
		/* [out] */ IUnknown **ppObject);

	STDMETHODIMP CancelObjectCreation(IUnknown *pIUnknownCancelCookie);
	STDMETHODIMP GetMaxNumberOfBytesRequiredForResolution(QWORD *pqwBytes);
protected:
	//创建 MediaSource
	virtual void OnCreateMediaSource(IMFByteStream* byteStream, IMFAsyncCallback *callback,
		IUnknown* unkState) = 0;
};

NSED_TOMATO_MEDIA