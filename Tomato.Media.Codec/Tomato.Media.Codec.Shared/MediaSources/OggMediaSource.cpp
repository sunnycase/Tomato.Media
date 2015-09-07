//
// Tomato Media Codec
// Media Foundation Ogg MediaSource
// 
// 作者：SunnyCase
// 创建时间：2015-09-06
#include "pch.h"
#include "OggMediaSource.h"
#include "libvorbis/codec.h"
#include "libtheora/theoradec.h"
#include "constants.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace concurrency;

enum : long
{
	SizePerReadPeriod = 64 * 1024	// 64 KB
};

#define LOCK_STATE() std::lock_guard<decltype(stateMutex)> locker(stateMutex)

namespace
{
	struct ReadBOSPacketContext : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IUnknown>
	{
		task_completion_event<void> ReadCompletionEvent;
		ComPtr<IMFAsyncCallback> ReadCallback;
	};

	struct TheoraStreamDetector
	{
		static const char magic[7];

		static bool IsMyHeader(const ogg_stream_state& streamState)
		{
			if (streamState.body_fill - streamState.body_returned < sizeof(magic)) return false;
			return memcmp(streamState.body_data + streamState.body_returned, magic, sizeof(magic)) == 0;
		}

		static ComPtr<IMFStreamDescriptor> BuildStreamDescriptor(const ogg_stream_state& streamState)
		{
			auto myStreamState = streamState;

			th_info info;
			th_info_init(&info);

			ogg_packet packet;
			if (ogg_stream_packetout(&myStreamState, &packet) != 1) ThrowIfFailed(MF_E_INVALID_STREAM_DATA);
			if (th_decode_headerin(&info, nullptr, nullptr, &packet) < 0) ThrowIfFailed(MF_E_INVALID_STREAM_DATA);

			ComPtr<IMFMediaType> mediaType;
			ThrowIfFailed(MFCreateMediaType(&mediaType));
			ThrowIfFailed(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
			ThrowIfFailed(mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_Theora));
			ThrowIfFailed(MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, info.frame_width, info.frame_height));
			ThrowIfFailed(MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, info.fps_numerator, info.fps_denominator));

			ComPtr<IMFStreamDescriptor> desc;
			ThrowIfFailed(MFCreateStreamDescriptor(DWORD(streamState.serialno), 1,
				mediaType.GetAddressOf(), &desc));

			return desc;
		}
	};
	const char TheoraStreamDetector::magic[7] = { '\x80', 't', 'h', 'e', 'o', 'r', 'a' };
}

OggMediaSource::OggMediaSource()
	:workerQueue(MFASYNC_CALLBACK_QUEUE_UNDEFINED),
	byteStreamReadCompletedCallback(Make < MFAsyncCallbackWithWeakRef < OggMediaSource,
		MediaSourceBase >> (AsWeak(), &OggMediaSource::OnByteStreamReadCompleted))
{
}

OggMediaSource::~OggMediaSource()
{
	for (auto&& it : streamStates)
		ogg_stream_clear(&it.second);
	streamStates.clear();
	ogg_sync_clear(&syncState);
}

DWORD OggMediaSource::OnGetCharacteristics()
{
	return 0;
}

void OggMediaSource::OnValidatePresentationDescriptor(IMFPresentationDescriptor * pPD)
{
}

concurrency::task<WRL::ComPtr<IMFPresentationDescriptor>> OggMediaSource::OnCreatePresentationDescriptor(IMFByteStream * stream)
{
	byteStream = stream;
	ThrowIfNot(ogg_sync_init(&syncState) == 0, L"Cannot init ogg sync state.");
	RegisterWorkThreadIfNeeded();

	ComPtr<OggMediaSource> thisGuard(this);
	return ReadAllBOSPackets().then([thisGuard, this]
	{
		return BuildPresentationDescriptor();
	});
}

concurrency::task<void> OggMediaSource::OnStreamsRequestData(TOperation & op)
{
	return concurrency::task<void>();
}

void OggMediaSource::OnStartStream(DWORD streamId, bool selected, MFTIME position)
{
}

void OggMediaSource::OnPauseStream(DWORD streamId)
{
}

void OggMediaSource::OnStopStream(DWORD streamId)
{
}

void OggMediaSource::OnByteStreamReadCompleted(IMFAsyncResult* asyncResult)
{
}

void OggMediaSource::OnByteStreamReadCompletedForBOS(IMFAsyncResult * asyncResult)
{
	auto context = static_cast<ReadBOSPacketContext*>(asyncResult->GetStateNoAddRef());
	try
	{
		ULONG read;
		ThrowIfFailed(byteStream->EndRead(asyncResult, &read));
		ThrowIfFailed(ogg_sync_wrote(&syncState, static_cast<long>(read)));

		// 是否继续取出 Page
		bool bContinuePageOut;
		do
		{
			bContinuePageOut = false;
			ogg_page page;
			// 没有取出 Page
			if (ogg_sync_pageout(&syncState, &page) != 1)
			{
				// 如果读取到的字节数少于请求的，说明已经到了流的末尾
				if (read < SizePerReadPeriod)
				{
					// 完成头部读取
					context->ReadCompletionEvent.set();
				}
				// 否则请求更多字节
				else
					SyncBufferFromByteStream(context->ReadCallback.Get(), context);
			}
			// 取出了 Page
			else
			{
				auto serialno = ogg_page_serialno(&page);
				if (ogg_page_bos(&page))
				{
					bContinuePageOut = true;
					auto streamIt = streamStates.find(serialno);
					// 发现新的流
					if (streamIt == streamStates.end())
					{
						streamIt = streamStates.emplace(serialno, ogg_stream_state()).first;
						// 初始化流
						ThrowIfNot(ogg_stream_init(&streamIt->second, serialno) == 0, L"Cannot init ogg stream state.");
					}
					if (ogg_stream_pagein(&streamIt->second, &page) != 0)
					{
						// 忽略错误
						OutputDebugString(L"ogg page in stream occurred error.");
					}
				}
				else
				{
					auto streamIt = streamStates.find(serialno);
					if (streamIt != streamStates.end())
					{
						if (ogg_stream_pagein(&streamIt->second, &page) != 0)
						{
							// 忽略错误
							OutputDebugString(L"ogg page in stream occurred error.");
						}
					}
					else
					{
						// 忽略错误
						OutputDebugString(L"invalid stream serialno.");
					}
					// 完成头部读取
					context->ReadCompletionEvent.set();
				}
			}
		} while (bContinuePageOut);
	}
	CATCH_ALL_WITHEVENT(context->ReadCompletionEvent);
}

task<void> OggMediaSource::ReadAllBOSPackets()
{
	auto context = Make<ReadBOSPacketContext>();
	context->ReadCallback = Make < MFAsyncCallbackWithWeakRef < OggMediaSource,
		MediaSourceBase >> (AsWeak(), &OggMediaSource::OnByteStreamReadCompletedForBOS);
	SyncBufferFromByteStream(context->ReadCallback.Get(), context.Get());
	return create_task(context->ReadCompletionEvent);
}

void OggMediaSource::SyncBufferFromByteStream(IMFAsyncCallback* asyncCallback, IUnknown* state)
{
	auto buffer = ogg_sync_buffer(&syncState, SizePerReadPeriod);
	ThrowIfNot(buffer, L"Cannot create ogg sync buffer.");
	ThrowIfFailed(byteStream->BeginRead(reinterpret_cast<BYTE*>(buffer), SizePerReadPeriod,
		asyncCallback, state));
}

WRL::ComPtr<IMFPresentationDescriptor> OggMediaSource::BuildPresentationDescriptor()
{
	std::vector<ComPtr<IMFStreamDescriptor>> streamDescriptors;
	for (auto it = streamStates.begin();it != streamStates.end();++it)
	{
		auto&& streamState = it->second;
		ComPtr<IMFStreamDescriptor> streamDescriptor;

		if (TheoraStreamDetector::IsMyHeader(streamState))
			streamDescriptor = TheoraStreamDetector::BuildStreamDescriptor(streamState);

		// 没有识别出流，则抛弃
		if (!streamDescriptor)
		{
			ogg_stream_clear(&streamState);
			it = streamStates.erase(it);
		}
		else
			streamDescriptors.emplace_back(std::move(streamDescriptor));
	}
	// 没有一个可识别的流，则引发异常
	if (streamDescriptors.empty())
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	ComPtr<IMFPresentationDescriptor> presentationDescriptor;
	ThrowIfFailed(MFCreatePresentationDescriptor(streamDescriptors.size(), reinterpret_cast<
		IMFStreamDescriptor**>(streamDescriptors.data()), &presentationDescriptor));
	ThrowIfFailed(presentationDescriptor->SelectStream(0));
	return presentationDescriptor;
}

void OggMediaSource::RegisterWorkThreadIfNeeded()
{
	if (workThreadRegistered) return;

	if (!workerQueue.IsValid())
		workerQueue = MFWorkerQueueProvider::GetAudio();

	auto weak(AsWeak());

	workThreadRegistered = true;
}

void OggMediaSource::EndOfSource()
{

}
