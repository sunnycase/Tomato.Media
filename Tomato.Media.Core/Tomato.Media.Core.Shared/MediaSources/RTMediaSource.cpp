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
#include "Metadata/id3v2.h"

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
	return stream->CloneStream();
}

task<void> RTMediaSource::Initialize()
{
	if (!initialized)
	{
		initialized = true;
		return FillBriefMediaMetadatas(std::shared_ptr<MediaMetadataContainer>(&metadatas, [](MediaMetadataContainer*) {}));
	}
	return task_from_result();
}

concurrency::task<void> RTMediaSource::InitializeFullMetadatas()
{
	return FillFullMediaMetadatas(std::shared_ptr<MediaMetadataContainer>(&metadatas, [](MediaMetadataContainer*) {}));
}

const MediaMetadataContainer & RTMediaSource::GetMetadatas() const
{
	return metadatas;
}

#define BUFFER_SIZE 2 * 1024 * 1024

int64_t RTMediaSource::GetDuration()
{
	if (duration == -1)
	{
		MFAVIOContext ioctx(CreateMFByteStream(), BUFFER_SIZE, false);
		duration = MediaMetadataHelper::GetDuration(ioctx.Get());
	}
	return duration;
}

MEDIA_CORE_API std::unique_ptr<IMediaSource> __stdcall NS_TOMATO_MEDIA::CreateRTMediaSource(
	Windows::Storage::Streams::IRandomAccessStream^ stream)
{
	return std::make_unique<RTMediaSource>(stream);
}

task<void> IMediaSourceIntern::FillBriefMediaMetadatas(std::shared_ptr<MediaMetadataContainer> container)
{
	// ID3V2
	return ID3V2Meta::ReadBriefMetadata(this, container)
		.then([=](bool good)
	{
		if (good && container->GetSize() == 7)
			return task_from_result();
		// ID3V1
		return ID3V1Meta::ReadMetadata(this, container)
			.then([=](bool good)
		{
			if (!good)
			{
				// FFmpeg
				MFAVIOContext ioctx(CreateMFByteStream(), BUFFER_SIZE, false);
				MediaMetadataHelper::FillMediaMetadatas(ioctx.Get(), *container);
			}
		});
	});
}

concurrency::task<void> IMediaSourceIntern::FillFullMediaMetadatas(std::shared_ptr<MediaMetadataContainer> container)
{
	// ID3V2
	return ID3V2Meta::ReadExtraMetadata(this, container)
		.then([=](bool good)
	{
	});
}
