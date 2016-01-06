//
// Tomato Media
// IMFMediaSource 包装
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#pragma once
#include "common.h"
#include <ppltasks.h>
#include <mfidl.h>

DEFINE_NS_MEDIA

class MFMediaSourceFactory : public std::enable_shared_from_this<MFMediaSourceFactory>
{
public:
	MFMediaSourceFactory();

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	void Open(IStream* stream, const std::wstring& uriHint);
#endif

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#if __cplusplus_winrt
	concurrency::task<void> OpenAsync(Windows::Storage::Streams::IRandomAccessStream^ stream, Platform::String^ uriHint);
#endif
#endif

	void Open(IMFByteStream* byteStream, const std::wstring& uriHint);
	concurrency::task<void> OpenAsync(IMFByteStream* byteStream, const std::wstring& uriHint);

	void Reset();
	IMFMediaSource* Get() const noexcept { return _mediaSource.Get(); }

	DEFINE_PROPERTY_GET(Title, std::wstring);
	std::wstring get_Title();

	DEFINE_PROPERTY_GET(Album, std::wstring);
	std::wstring get_Album();

	DEFINE_PROPERTY_GET(Artist, std::wstring);
	std::wstring get_Artist();

	DEFINE_PROPERTY_GET(AlbumArtist, std::wstring);
	std::wstring get_AlbumArtist();

	DEFINE_PROPERTY_GET(Lyrics, std::wstring);
	std::wstring get_Lyrics();

	DEFINE_PROPERTY_GET(Duration, MFTIME);
	MFTIME get_Duration();

	HRESULT OpenAsyncCallback(IMFAsyncResult *pAsyncResult);
private:
	// 载入元数据
	void EnsureInitializeMetadata();
	void CheckOpened() const;
	std::wstring ReadStringMetadata(LPCWSTR key);
private:
	WRL::ComPtr<IMFMediaSource> _mediaSource;
	WRL::ComPtr<IMFMetadata> metadata;
	WRL::ComPtr<IMFPresentationDescriptor> pd;
};

END_NS_MEDIA
