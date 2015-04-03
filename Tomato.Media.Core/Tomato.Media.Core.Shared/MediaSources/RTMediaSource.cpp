//
// Tomato Media
// Windows Runtime 媒体源
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#include "pch.h"
#include "RTMediaSource.h"
#include "Utilities/libavhelpers.h"
#include "Metadata/id3v1.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace Windows::Storage::Streams;
using namespace wrl;
using namespace concurrency;

struct MFRegistry
{
	MFRegistry()
	{
		THROW_IF_FAILED(MFStartup(MF_SDK_VERSION, MFSTARTUP_LITE));
	}

	~MFRegistry()
	{
		MFShutdown();
	}
};

RTMediaSource::RTMediaSource(IRandomAccessStream ^ stream)
	:stream(stream)
{
	static MFRegistry reg;
}

ComPtr<IMFByteStream> RTMediaSource::CreateMFByteStream()
{
	ComPtr<IMFByteStream> byteStream;
	THROW_IF_FAILED(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &byteStream));

	return std::move(byteStream);
}

IRandomAccessStream ^ RTMediaSource::CreateRTRandomAccessStream()
{
	return stream;
}

task<void> RTMediaSource::Initialize()
{
	if (!initialized)
	{
		MFAVIOContext ioctx(CreateMFByteStream(), 4096, false);
		MediaMetadataHelper::FillMediaMetadatas(ioctx.Get(), metadatas);

		initialized = true;
	}
	auto meta = std::make_shared<ID3V1Meta>();
	return create_task(meta->Read(this))
		.then([=](bool good)
	{

	});
}

const MediaMetadataContainer & RTMediaSource::GetMetadatas() const
{
	return metadatas;
}

int64_t RTMediaSource::GetDuration()
{
	if (duration == -1)
	{
		MFAVIOContext ioctx(CreateMFByteStream(), 4096, false);
		duration = MediaMetadataHelper::GetDuration(ioctx.Get());
	}
	return duration;
}

MEDIA_CORE_API std::unique_ptr<IMediaSource> __stdcall NS_TOMATO_MEDIA::CreateRTMediaSource(
	Windows::Storage::Streams::IRandomAccessStream^ stream)
{
	return std::make_unique<RTMediaSource>(stream);
}