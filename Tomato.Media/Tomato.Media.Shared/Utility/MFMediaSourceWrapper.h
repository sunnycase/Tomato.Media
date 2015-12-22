//
// Tomato Media
// IMFMediaSource 包装
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#pragma once
#include "common.h"

DEFINE_NS_MEDIA

class MFMediaSourceWrapper : public WRL::ComPtr<IMFMediaSource>
{
public:
	MFMediaSourceWrapper();

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

	DEFINE_PROPERTY_GET(Title, std::wstring);
	std::wstring get_Title() const;

	DEFINE_PROPERTY_GET(Album, std::wstring);
	std::wstring get_Album() const;

	DEFINE_PROPERTY_GET(Artist, std::wstring);
	std::wstring get_Artist() const;

	DEFINE_PROPERTY_GET(AlbumArtist, std::wstring);
	std::wstring get_AlbumArtist() const;

	DEFINE_PROPERTY_GET(Lyrics, std::wstring);
	std::wstring get_Lyrics() const;

	DEFINE_PROPERTY_GET(Duration, MFTIME);
	MFTIME get_Duration() const;
private:
	// 载入元数据
	void EnsureInitializeMetadata() const;
	void CheckOpened() const;
private:
	mutable WRL::ComPtr<IMFMetadata> metadata;
	mutable WRL::ComPtr<IMFPresentationDescriptor> pd;
};

END_NS_MEDIA
