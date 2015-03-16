//
// Tomato Media
// Windows Runtime 媒体源
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#include "pch.h"
#include "RTMediaSource.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace Windows::Storage::Streams;
using namespace wrl;
using namespace concurrency;

RTMediaSource::RTMediaSource(IRandomAccessStream ^ stream)
	:stream(stream)
{
}

ComPtr<IMFByteStream> RTMediaSource::CreateMFByteStream()
{
	ComPtr<IMFByteStream> byteStream;
	THROW_IF_FAILED(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &byteStream));

	return std::move(byteStream);
}

MEDIA_CORE_API std::unique_ptr<IMediaSource> __stdcall NS_TOMATO_MEDIA::CreateRTMediaSource(
	Windows::Storage::Streams::IRandomAccessStream^ stream)
{
	return std::make_unique<RTMediaSource>(stream);
}