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

void MFMediaSourceWrapper::Open(IStream * stream, const std::wstring& uriHint)
{
	Reset();

	ComPtr<IMFByteStream> byteStream;
	ThrowIfFailed(MFCreateMFByteStreamOnStream(stream, &byteStream));
	Open(byteStream.Get(), uriHint);
}

#endif

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#if __cplusplus_winrt

using namespace Windows::Storage::Streams;

task<void> MFMediaSourceWrapper::OpenAsync(IRandomAccessStream ^ stream, Platform::String^ uriHint)
{
	InternalRelease();
	metadata.Reset();

	ComPtr<IMFByteStream> byteStream;
	ThrowIfFailed(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &byteStream));

	return OpenAsync(byteStream.Get(), std::wstring(uriHint->Begin(), uriHint->End()));
}

#endif
#endif

void MFMediaSourceWrapper::Open(IMFByteStream* byteStream, const std::wstring& uriHint)
{
	Reset();

	ComPtr<IMFSourceResolver> sourceResolver;
	ThrowIfFailed(MFCreateSourceResolver(&sourceResolver));

	MF_OBJECT_TYPE objType;
	ComPtr<IUnknown> unkMediaSource;
	DWORD flag = MF_RESOLUTION_MEDIASOURCE;
	if (uriHint.empty())
		flag |= MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE;
	ThrowIfFailed(sourceResolver->CreateObjectFromByteStream(byteStream, uriHint.c_str(), flag, nullptr, &objType, &unkMediaSource));
	if (objType != MF_OBJECT_MEDIASOURCE)
		ThrowIfFailed(E_FAIL, L"Cannot create media source.");
	ThrowIfFailed(unkMediaSource.As(this));
}

task<void> MFMediaSourceWrapper::OpenAsync(IMFByteStream* byteStream, const std::wstring& uriHint)
{
	Reset();

	ComPtr<IMFSourceResolver> sourceResolver;
	ThrowIfFailed(MFCreateSourceResolver(&sourceResolver));

	auto callback = Make<MFAsyncTaskCallback>();
	ComPtr<IUnknown> cancelCookie;
	DWORD flag = MF_RESOLUTION_MEDIASOURCE;
	if (uriHint.empty())
		flag |= MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE;
	ThrowIfFailed(sourceResolver->BeginCreateObjectFromByteStream(byteStream, uriHint.c_str(), flag, nullptr, &cancelCookie, callback.Get(), nullptr));

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

std::wstring MFMediaSourceWrapper::ReadStringMetadata(LPCWSTR key) const
{
	EnsureInitializeMetadata();

	PROPVARIANT variant;
	if (FAILED(metadata->GetProperty(key, &variant)))
		return{};
	if (variant.vt & VT_LPWSTR)
	{
		if (variant.vt & VT_VECTOR)
		{
			std::wstring value;
			auto size = variant.calpwstr.cElems;
			auto ptr = reinterpret_cast<LPWSTR*>(variant.calpwstr.pElems);
			for (size_t i = 0; i < size; i++)
			{
				if (i != 0)
					value += L';';
				value.append(ptr[i]);
			}
			PropVariantClear(&variant);
			return value;
		}
		else
		{
			std::wstring value(variant.pwszVal);
			PropVariantClear(&variant);
			return value;
		}
	}
	else
	{
		PropVariantClear(&variant);
		return{};
	}
}

std::wstring MFMediaSourceWrapper::get_Title() const
{
	return ReadStringMetadata(L"Title");
}

std::wstring MFMediaSourceWrapper::get_Album() const
{
	return ReadStringMetadata(L"WM/AlbumTitle");
}

std::wstring MFMediaSourceWrapper::get_Artist() const
{
	return ReadStringMetadata(L"Author");
}

std::wstring MFMediaSourceWrapper::get_AlbumArtist() const
{
	return ReadStringMetadata(L"WM/AlbumArtist");
}

std::wstring MFMediaSourceWrapper::get_Lyrics() const
{
	return ReadStringMetadata(L"WM/Lyrics");
}

MFTIME MFMediaSourceWrapper::get_Duration() const
{
	EnsureInitializeMetadata();

	MFTIME duration;
	if (FAILED(pd->GetUINT64(MF_PD_DURATION, reinterpret_cast<UINT64*>(&duration))))
		return -1;
	return duration;
}

void MFMediaSourceWrapper::Reset()
{
	ComPtr::Reset();
	metadata.Reset();
}