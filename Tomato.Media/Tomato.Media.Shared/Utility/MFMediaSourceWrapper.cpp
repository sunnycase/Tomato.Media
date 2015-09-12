//
// Tomato Media
// IMFMediaSource 包装
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#include "pch.h"
#include "MFMediaSourceWrapper.h"
#include "../../include/MFAsyncCallback.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace WRL;
using namespace concurrency;

MFMediaSourceWrapper::MFMediaSourceWrapper()
{

}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

void MFMediaSourceWrapper::Open(IStream * stream)
{
	Reset();

	ComPtr<IMFByteStream> byteStream;
	ThrowIfFailed(MFCreateMFByteStreamOnStream(stream, &byteStream));
	Open(byteStream.Get());
}

#endif

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#if __cplusplus_winrt

using namespace Windows::Storage::Streams;

task<void> MFMediaSourceWrapper::OpenAsync(IRandomAccessStream ^ stream)
{
	InternalRelease();
	metadata.Reset();

	ComPtr<IMFByteStream> byteStream;
	ThrowIfFailed(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &byteStream));
	
	return OpenAsync(byteStream.Get());
}

#endif
#endif

void MFMediaSourceWrapper::Open(IMFByteStream* byteStream)
{
	Reset();

	ComPtr<IMFSourceResolver> sourceResolver;
	ThrowIfFailed(MFCreateSourceResolver(&sourceResolver));
	
	MF_OBJECT_TYPE objType;
	ComPtr<IUnknown> unkMediaSource;
	ThrowIfFailed(sourceResolver->CreateObjectFromByteStream(byteStream, nullptr, 
		MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE, nullptr, &objType, &unkMediaSource));
	if (objType != MF_OBJECT_MEDIASOURCE)
		ThrowIfFailed(E_FAIL, L"Cannot create media source.");
	ThrowIfFailed(unkMediaSource.As(this));
}

task<void> MFMediaSourceWrapper::OpenAsync(IMFByteStream* byteStream)
{
	Reset();

	ComPtr<IMFSourceResolver> sourceResolver;
	ThrowIfFailed(MFCreateSourceResolver(&sourceResolver));

	auto callback = Make<MFAsyncTaskCallback>();
	ComPtr<IUnknown> cancelCookie;
	ThrowIfFailed(sourceResolver->BeginCreateObjectFromByteStream(byteStream, nullptr,
		MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE, nullptr, &cancelCookie, callback.Get(), nullptr));

	return create_task(callback->GetEvent()).then([=](ComPtr<IMFAsyncResult> result)
	{
		MF_OBJECT_TYPE objType;

		ComPtr<IUnknown> unkMediaSource;
		ThrowIfFailed(sourceResolver->EndCreateObjectFromByteStream(result.Get(), &objType, &unkMediaSource));
		if (objType != MF_OBJECT_MEDIASOURCE)
			ThrowIfFailed(E_FAIL, L"Cannot create media source.");
		ThrowIfFailed(unkMediaSource.As(this));
	});
}

void MFMediaSourceWrapper::EnsureInitializeMetadata() const
{
	if (!metadata)
	{
		CheckOpened();

		ComPtr<IMFPresentationDescriptor> pd;
		ComPtr<IMFMetadataProvider> provider;

		ThrowIfFailed(ptr_->CreatePresentationDescriptor(&pd));
		ThrowIfFailed(MFGetService(ptr_, MF_METADATA_PROVIDER_SERVICE, IID_PPV_ARGS(&provider)));
		ThrowIfFailed(provider->GetMFMetadata(pd.Get(), 0, 0, metadata.ReleaseAndGetAddressOf()));
	}
}

void MFMediaSourceWrapper::CheckOpened() const
{
	if (!ptr_)
		ThrowIfFailed(E_NOT_VALID_STATE, L"Must open the media source firstly.");
}

std::wstring MFMediaSourceWrapper::get_Title() const
{
	EnsureInitializeMetadata();

	PROPVARIANT variant;
	ThrowIfFailed(metadata->GetProperty(L"Title", &variant));
	std::wstring value(variant.pwszVal);
	PropVariantClear(&variant);
	return value;
}

std::wstring MFMediaSourceWrapper::get_Album() const
{
	EnsureInitializeMetadata();

	PROPVARIANT variant;
	ThrowIfFailed(metadata->GetProperty(L"WM/AlbumTitle", &variant));
	std::wstring value(variant.pwszVal);
	PropVariantClear(&variant);
	return value;
}

void MFMediaSourceWrapper::Reset()
{
	ComPtr::Reset();
	metadata.Reset();
}