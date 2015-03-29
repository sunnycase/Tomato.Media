//
// Tomato Media
// Windows Runtime 媒体源
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#include "pch.h"
#include "RTMediaSource.h"
#include "Utilities/libavhelpers.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace Windows::Storage::Streams;
using namespace wrl;
using namespace concurrency;

struct MFTRegistry
{
	MFTRegistry()
	{
		THROW_IF_FAILED(MFStartup(MF_SDK_VERSION, MFSTARTUP_LITE));
	}

	~MFTRegistry()
	{
		MFShutdown();
	}
};

RTMediaSource::RTMediaSource(IRandomAccessStream ^ stream)
	:stream(stream)
{
	static MFTRegistry reg;
}

ComPtr<IMFByteStream> RTMediaSource::CreateMFByteStream()
{
	ComPtr<IMFByteStream> byteStream;
	THROW_IF_FAILED(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &byteStream));

	return std::move(byteStream);
}

task<void> RTMediaSource::Initialize()
{
	if (!initialized)
	{
		MFAVIOContext ioctx(CreateMFByteStream(), 4096, false);
		MediaMetadataHelper::FillMediaMetadatas(ioctx.Get(), metadatas);

		initialized = true;
	}
	return task_from_result();
}

const MediaMetadataContainer & RTMediaSource::GetMetadatas() const
{
	return metadatas;
}

MEDIA_CORE_API std::unique_ptr<IMediaSource> __stdcall NS_TOMATO_MEDIA::CreateRTMediaSource(
	Windows::Storage::Streams::IRandomAccessStream^ stream)
{
	return std::make_unique<RTMediaSource>(stream);
}