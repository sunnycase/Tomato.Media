//
// Tomato Media
// IMFMediaSource 包装
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#include "pch.h"
#include "MFMediaSourceFactory.h"
#include "../../include/MFAsyncCallback.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace WRL;
using namespace concurrency;

MFMediaSourceFactory::MFMediaSourceFactory()
{

}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

void MFMediaSourceFactory::Open(IStream * stream, const std::wstring& uriHint)
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

task<void> MFMediaSourceFactory::OpenAsync(IRandomAccessStream ^ stream, Platform::String^ uriHint)
{
	Reset();

	ComPtr<IMFByteStream> byteStream;
	ThrowIfFailed(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &byteStream));

	return OpenAsync(byteStream.Get(), std::wstring(uriHint->Begin(), uriHint->End()));
}

#endif
#endif

void MFMediaSourceFactory::Open(IMFByteStream* byteStream, const std::wstring& uriHint)
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
	ThrowIfFailed(unkMediaSource.As(&_mediaSource));
}

task<void> MFMediaSourceFactory::OpenAsync(IMFByteStream* byteStream, const std::wstring& uriHint)
{
	Reset();

	ComPtr<IMFSourceResolver> sourceResolver;
	ThrowIfFailed(MFCreateSourceResolver(&sourceResolver));

	auto callback = Make<MFAsyncTaskCallback<MFMediaSourceFactory>>(shared_from_this(), &MFMediaSourceFactory::OpenAsyncCallback);
	ComPtr<IUnknown> cancelCookie;
	DWORD flag = MF_RESOLUTION_MEDIASOURCE;
	if (uriHint.empty())
		flag |= MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE;
	ThrowIfFailed(sourceResolver->BeginCreateObjectFromByteStream(byteStream, uriHint.c_str(),
		flag, nullptr, &cancelCookie, callback.Get(), sourceResolver.Get()));

	return create_task(callback->GetEvent());
}

HRESULT MFMediaSourceFactory::OpenAsyncCallback(IMFAsyncResult *pAsyncResult)
{
	try
	{
		auto sourceResolver = static_cast<IMFSourceResolver*>(pAsyncResult->GetStateNoAddRef());
		MF_OBJECT_TYPE objType;

		ComPtr<IUnknown> unkMediaSource;
		ThrowIfFailed(sourceResolver->EndCreateObjectFromByteStream(pAsyncResult, &objType, &unkMediaSource));
		if (objType != MF_OBJECT_MEDIASOURCE)
			ThrowIfFailed(E_FAIL, L"Cannot create media source.");
		ThrowIfFailed(unkMediaSource.As(&_mediaSource));
		return S_OK;
	}
	CATCH_ALL();
}

void MFMediaSourceFactory::EnsureInitializeMetadata()
{
	if (!metadata)
	{
		CheckOpened();

		ComPtr<IMFMetadataProvider> provider;

		ThrowIfFailed(_mediaSource->CreatePresentationDescriptor(&pd));
		ThrowIfFailed(MFGetService(_mediaSource.Get(), MF_METADATA_PROVIDER_SERVICE, IID_PPV_ARGS(&provider)));
		ThrowIfFailed(provider->GetMFMetadata(pd.Get(), 0, 0, metadata.ReleaseAndGetAddressOf()));
	}
}

void MFMediaSourceFactory::CheckOpened() const
{
	if (!_mediaSource)
		ThrowIfFailed(E_NOT_VALID_STATE, L"Must open the media source firstly.");
}

std::wstring MFMediaSourceFactory::ReadStringMetadata(LPCWSTR key)
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

std::wstring MFMediaSourceFactory::get_Title()
{
	return ReadStringMetadata(L"Title");
}

std::wstring MFMediaSourceFactory::get_Album()
{
	return ReadStringMetadata(L"WM/AlbumTitle");
}

std::wstring MFMediaSourceFactory::get_Artist()
{
	return ReadStringMetadata(L"Author");
}

std::wstring MFMediaSourceFactory::get_AlbumArtist()
{
	return ReadStringMetadata(L"WM/AlbumArtist");
}

std::wstring MFMediaSourceFactory::get_Lyrics()
{
	return ReadStringMetadata(L"WM/Lyrics");
}

MFTIME MFMediaSourceFactory::get_Duration()
{
	EnsureInitializeMetadata();

	MFTIME duration;
	if (FAILED(pd->GetUINT64(MF_PD_DURATION, reinterpret_cast<UINT64*>(&duration))))
		return -1;
	return duration;
}

void MFMediaSourceFactory::Reset()
{
	_mediaSource.Reset();
	metadata.Reset();
}