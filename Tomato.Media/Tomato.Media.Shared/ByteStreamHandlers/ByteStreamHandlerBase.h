//
// Tomato Media Codec
// Media Foundation ByteStream Handler 基类
// 
// 作者：SunnyCase
// 创建时间：2015-03-17
#pragma once
#include <Tomato.Media/Tomato.Media.h>
#include <mfidl.h>
#include <windows.media.h>

DEFINE_NS_MEDIA_CODEC

struct ByteStreamHandlerRegisterInfo
{
	LPCTSTR FileExtension;
	LPCTSTR MimeType;
};

class ByteStreamHandlerBase : public WRL::RuntimeClass<WRL::RuntimeClassFlags<
#if (NTDDI_VERSION >= NTDDI_WIN8)
	WRL::WinRtClassicComMix>,
	ABI::Windows::Media::IMediaExtension,
#else
	WRL::ClassicCom>,
#endif
	IMFByteStreamHandler>
{
public:
	ByteStreamHandlerBase();
	virtual ~ByteStreamHandlerBase();

#if (NTDDI_VERSION >= NTDDI_WIN8)
	// IMediaExtension
	IFACEMETHOD(SetProperties) (ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration) override;
#endif

	// IMFByteStreamHandler
	//从字节流创建 APESource
	STDMETHODIMP BeginCreateObject(
		/* [in] */ IMFByteStream *pByteStream,
		/* [in] */ LPCWSTR pwszURL,
		/* [in] */ DWORD dwFlags,
		/* [in] */ IPropertyStore *pProps,
		/* [out] */ IUnknown **ppIUnknownCancelCookie,
		/* [in] */ IMFAsyncCallback *pCallback,
		/* [in] */ IUnknown *punkState) override;

	STDMETHODIMP EndCreateObject(
		/* [in] */ IMFAsyncResult *pResult,
		/* [out] */ MF_OBJECT_TYPE *pObjectType,
		/* [out] */ IUnknown **ppObject) override;

	STDMETHODIMP CancelObjectCreation(IUnknown *pIUnknownCancelCookie) override;
	STDMETHODIMP GetMaxNumberOfBytesRequiredForResolution(QWORD *pqwBytes) override;
protected:
	//创建 MediaSource
	virtual void OnCreateMediaSource(IMFByteStream* byteStream, LPCWSTR pwszURL, IMFAsyncCallback *callback,
		IUnknown* unkState) = 0;
};

END_NS_MEDIA_CODEC