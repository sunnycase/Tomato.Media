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
#include "libvorbis/codec.h"
#include "libvorbis/vorbis_raii.h"
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
#define INITGUID
#include <guiddef.h>
	// {36062527-1905-4F37-8900-11A94EB2C1B1}	MF_PD_TM_BOSContext
	// ReadBOSPacketContext
	DEFINE_GUID(MF_PD_TM_BOSContext,
		0x36062527, 0x1905, 0x4f37, 0x89, 0x0, 0x11, 0xa9, 0x4e, 0xb2, 0xc1, 0xb1);
#undef INITGUID

	struct ReadBOSPacketContext : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IUnknown>
	{
		task_completion_event<void> ReadCompletionEvent;
		ComPtr<IMFAsyncCallback> ReadCallback;
		std::map<int, ogg_stream_state> StreamStates;

		virtual ~ReadBOSPacketContext()
		{
			for (auto&& it : StreamStates)
				ogg_stream_clear(&it.second);
			StreamStates.clear();
		}
	};

	MFOffset MakeOffset(float v)
	{
		MFOffset offset;
		offset.value = short(v);
		offset.fract = WORD(65536 * (v - offset.value));
		return offset;
	}

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

			MFVideoArea area;
			area.OffsetX = MakeOffset(info.pic_x);
			area.OffsetY = MakeOffset(info.pic_y);
			area.Area.cx = info.pic_width;
			area.Area.cy = info.pic_height;

			ComPtr<IMFMediaType> mediaType;
			ThrowIfFailed(MFCreateMediaType(&mediaType));
			ThrowIfFailed(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
			ThrowIfFailed(mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_Theora));
			ThrowIfFailed(MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, info.frame_width, info.frame_height));
			ThrowIfFailed(MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, info.fps_numerator, info.fps_denominator));
			ThrowIfFailed(mediaType->SetUINT32(MF_MT_THEORA_PIXEL_FORMAT, info.pixel_fmt));
			ThrowIfFailed(mediaType->SetUINT32(MF_MT_PAN_SCAN_ENABLED, TRUE));
			ThrowIfFailed(MFSetAttributeRatio(mediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, info.aspect_numerator, info.aspect_denominator));
			ThrowIfFailed(mediaType->SetBlob(MF_MT_PAN_SCAN_APERTURE, reinterpret_cast<
				const UINT8*>(&area), sizeof(area)));

			ComPtr<IMFStreamDescriptor> desc;
			ThrowIfFailed(MFCreateStreamDescriptor(DWORD(streamState.serialno), 1,
				mediaType.GetAddressOf(), &desc));
			ComPtr<IMFMediaTypeHandler> mediaTypeHandler;
			ThrowIfFailed(desc->GetMediaTypeHandler(&mediaTypeHandler));
			ThrowIfFailed(mediaTypeHandler->SetCurrentMediaType(mediaType.Get()));

			return desc;
		}
	};
	const char TheoraStreamDetector::magic[7] = { '\x80', 't', 'h', 'e', 'o', 'r', 'a' };

	struct VorbisStreamDetector
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

			vorbis_info_raii info;

			ogg_packet packet;
			if (ogg_stream_packetout(&myStreamState, &packet) != 1) ThrowIfFailed(MF_E_INVALID_STREAM_DATA);
			if (vorbis_synthesis_idheader(&packet) != 1) ThrowIfFailed(MF_E_INVALID_STREAM_DATA);
			if (vorbis_synthesis_headerin(&info, nullptr, &packet) != 0) ThrowIfFailed(MF_E_INVALID_STREAM_DATA);

			ComPtr<IMFMediaType> mediaType;
			ThrowIfFailed(MFCreateMediaType(&mediaType));
			ThrowIfFailed(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
			ThrowIfFailed(mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Vorbis));
			ThrowIfFailed(mediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, info.channels));
			ThrowIfFailed(mediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, info.rate));
			if (info.bitrate_nominal)
			{
				ThrowIfFailed(mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, info.bitrate_nominal));
				// 固定码率
				if (info.bitrate_lower == info.bitrate_upper)
				{
					auto bitsPerSample = info.bitrate_nominal / info.rate / info.channels * 8;
					ThrowIfFailed(mediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample));
					ThrowIfFailed(mediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, bitsPerSample / 8 * info.channels));
				}
			}

			ComPtr<IMFStreamDescriptor> desc;
			ThrowIfFailed(MFCreateStreamDescriptor(DWORD(streamState.serialno), 1,
				mediaType.GetAddressOf(), &desc));
			ComPtr<IMFMediaTypeHandler> mediaTypeHandler;
			ThrowIfFailed(desc->GetMediaTypeHandler(&mediaTypeHandler));
			ThrowIfFailed(mediaTypeHandler->SetCurrentMediaType(mediaType.Get()));

			return desc;
		}
	};
	const char VorbisStreamDetector::magic[7] = { '\x01', 'v', 'o', 'r', 'b', 'i', 's' };

	class MetadataProvider : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFMetadataProvider>
	{
	public:
		MetadataProvider()
		{

		}

		// 通过 RuntimeClass 继承
		STDMETHODIMP GetMFMetadata(IMFPresentationDescriptor * pPresentationDescriptor, DWORD dwStreamIdentifier, DWORD dwFlags, IMFMetadata ** ppMFMetadata) override
		{
			return E_NOTIMPL;
		}
	};

	class RateControl : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IMFRateControl>
	{
	public:
		RateControl()
		{

		}

		// 通过 RuntimeClass 继承
		STDMETHODIMP SetRate(BOOL fThin, float flRate) override
		{
			thin = fThin;
			rate = flRate;
			return S_OK;
		}
		STDMETHODIMP GetRate(BOOL * pfThin, float * pflRate) override
		{
			*pfThin = thin;
			*pflRate = rate;
			return S_OK;
		}

		bool thin;
		float rate = 1.f;
	};

	ComPtr<IMFPresentationDescriptor> MakePresentationDescriptor(ReadBOSPacketContext* context)
	{
		std::vector<ComPtr<IMFStreamDescriptor>> streamDescriptors;
		for (auto it = context->StreamStates.begin();it != context->StreamStates.end();++it)
		{
			auto&& streamState = it->second;
			ComPtr<IMFStreamDescriptor> streamDescriptor;

			if (TheoraStreamDetector::IsMyHeader(streamState))
				streamDescriptor = TheoraStreamDetector::BuildStreamDescriptor(streamState);
			else if (VorbisStreamDetector::IsMyHeader(streamState))
				streamDescriptor = VorbisStreamDetector::BuildStreamDescriptor(streamState);

			// 识别出流，则加入 Descriptor
			if (streamDescriptor)
				streamDescriptors.emplace_back(std::move(streamDescriptor));
		}
		// 没有一个可识别的流，则引发异常
		if (streamDescriptors.empty())
			ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
		ComPtr<IMFPresentationDescriptor> presentationDescriptor;
		ThrowIfFailed(MFCreatePresentationDescriptor(streamDescriptors.size(), reinterpret_cast<
			IMFStreamDescriptor**>(streamDescriptors.data()), &presentationDescriptor));
		// 设置 Context 属性
		ThrowIfFailed(presentationDescriptor->SetUnknown(MF_PD_TM_BOSContext, context));
		ThrowIfFailed(presentationDescriptor->SelectStream(0));
		return presentationDescriptor;
	}

	struct ReadPageContext : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IUnknown>
	{
		task_completion_event<void> ReadCompletionEvent;
		ComPtr<IMFAsyncCallback> ReadCallback;
		int DesiredSerialNo;
	};
}

OggMediaSource::OggMediaSource()
	:workerQueue(MFASYNC_CALLBACK_QUEUE_UNDEFINED)
{

}

OggMediaSource::~OggMediaSource()
{
	ogg_sync_clear(&syncState);
}

HRESULT OggMediaSource::GetService(REFGUID guidService, REFIID riid, LPVOID * ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	auto hr = MediaSourceBase::GetService(guidService, riid, ppvObject);
	if (FAILED(hr))
	{
		if (guidService == MF_METADATA_PROVIDER_SERVICE)
		{
			auto provider = Make<MetadataProvider>();
			return provider.CopyTo(riid, ppvObject);
		}
	}
	return hr;
}

DWORD OggMediaSource::OnGetCharacteristics()
{
	return MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_CAN_SEEK;
}

void OggMediaSource::OnValidatePresentationDescriptor(IMFPresentationDescriptor * pPD)
{
	// 检出 Context 属性
	ComPtr<IUnknown> BOSContextUnk;
	ThrowIfFailed(pPD->GetUnknown(MF_PD_TM_BOSContext, IID_PPV_ARGS(&BOSContextUnk)));
	auto& context = static_cast<ReadBOSPacketContext&>(*BOSContextUnk.Get());

	DWORD streamCount;
	ThrowIfFailed(pPD->GetStreamDescriptorCount(&streamCount));
	for (DWORD i = 0; i < streamCount; i++)
	{
		BOOL selected;
		ComPtr<IMFStreamDescriptor> streamDescriptor;
		ThrowIfFailed(pPD->GetStreamDescriptorByIndex(i, &selected, &streamDescriptor));

		// 检测 streamDescriptor 的有效性
		DWORD streamId;
		ThrowIfFailed(streamDescriptor->GetStreamIdentifier(&streamId));
		auto streamStateIt = context.StreamStates.find(int(streamId));
		// 添加流
		if (selected)
		{
			if (streamStateIt != context.StreamStates.end())
			{
				AddStream(std::move(streamStateIt->second), streamDescriptor.Get());
				context.StreamStates.erase(streamStateIt);
			}
		}
	}
}

concurrency::task<WRL::ComPtr<IMFPresentationDescriptor>> OggMediaSource::OnCreatePresentationDescriptor(IMFByteStream * stream)
{
	byteStream = stream;
	ThrowIfNot(ogg_sync_init(&syncState) == 0, L"Cannot init ogg sync state.");
	RegisterWorkThreadIfNeeded();

	return BuildPresentationDescriptor();
}

concurrency::task<void> OggMediaSource::OnStreamsRequestData(IMFMediaStream* mediaStream)
{
	auto deliverStream = static_cast<OggDeliverMediaStream*>(mediaStream);
	if (deliverStream->DoesNeedMoreData())
	{
		auto desiredSerialno = deliverStream->GetSerialNo();
		{
			ogg_page page;
			int ret;
			// 读出了 Page
			while ((ret = ogg_sync_pageout(&syncState, &page)) == 1)
			{
				auto serialno = ogg_page_serialno(&page);
				// 读到了想要的流的 Page，结束读取
				if (serialno == desiredSerialno)
				{
					deliverStream->DeliverOggPage(page);
					return task_from_result();
				}
				else
				{
					auto streamIt = mediaStreams.find(serialno);
					if (streamIt != mediaStreams.end() && streamIt->second->IsActive())
						streamIt->second->DeliverOggPage(page);
				}
			}
		}
		// 没读出期待的 Page，则从 ByteStream 读取
		auto context = Make<ReadPageContext>();
		context->ReadCallback = Make < MFAsyncCallbackWithWeakRef < OggMediaSource,
			MediaSourceBase >> (AsWeak(), &OggMediaSource::OnByteStreamReadCompleted);
		context->DesiredSerialNo = desiredSerialno;
		SyncBufferFromByteStream(context->ReadCallback.Get(), context.Get());
		return create_task(context->ReadCompletionEvent);
	}
	return task_from_result();
}

void OggMediaSource::OnSeekSource(MFTIME position)
{
	QWORD offset;
	ThrowIfFailed(byteStream->Seek(msoBegin, 0, MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &offset));
}

void OggMediaSource::OnStartStream(DWORD streamId, bool selected, const PROPVARIANT& position)
{
	auto streamIt = mediaStreams.find(int(streamId));
	if (streamIt == mediaStreams.end())
		ThrowIfFailed(E_INVALIDARG);
	auto& stream = streamIt->second;

	// 判断流是不是已经启动了
	auto wasSelected = stream->IsActive();
	stream->SetIsActive(selected);
	if (selected)
		stream->Start(position);
	else if (wasSelected)
		stream->Stop();

	auto met = wasSelected ? MEUpdatedStream : MENewStream;

	ThrowIfFailed(eventQueue->QueueEventParamUnk(met, GUID_NULL, S_OK, static_cast<IMFMediaStream*>(stream.Get())));
}

void OggMediaSource::OnPauseStream(DWORD streamId)
{
	auto streamIt = mediaStreams.find(int(streamId));
	if (streamIt == mediaStreams.end())
		ThrowIfFailed(E_INVALIDARG);
	streamIt->second->Pause();
}

void OggMediaSource::OnStopStream(DWORD streamId)
{
	auto streamIt = mediaStreams.find(int(streamId));
	if (streamIt == mediaStreams.end())
		ThrowIfFailed(E_INVALIDARG);
	streamIt->second->Stop();
}

void OggMediaSource::OnByteStreamReadCompleted(IMFAsyncResult* asyncResult)
{
	auto context = static_cast<ReadPageContext*>(asyncResult->GetStateNoAddRef());
	try
	{
		ULONG read;
		ThrowIfFailed(byteStream->EndRead(asyncResult, &read));
		ThrowIfFailed(ogg_sync_wrote(&syncState, static_cast<long>(read)));

		ogg_page page;
		int ret;
		// 读出了 Page
		while ((ret = ogg_sync_pageout(&syncState, &page)) == 1)
		{
			auto serialno = ogg_page_serialno(&page);
			// 读到了想要的流的 Page，结束读取
			if (serialno == context->DesiredSerialNo)
			{
				auto streamIt = mediaStreams.find(serialno);
				ThrowIfNot(streamIt != mediaStreams.end(), L"Invalid stream request.");
				streamIt->second->DeliverOggPage(page);
				context->ReadCompletionEvent.set();
				return;
			}
			else
			{
				auto streamIt = mediaStreams.find(serialno);
				if (streamIt != mediaStreams.end() && streamIt->second->IsActive())
					streamIt->second->DeliverOggPage(page);
			}
		}
		// 没读出期待的 Page，继续 ByteStream 读取
		SyncBufferFromByteStream(context->ReadCallback.Get(), context);
	}
	CATCH_ALL_WITHEVENT(context->ReadCompletionEvent);
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
					auto streamIt = context->StreamStates.find(serialno);
					// 发现新的流
					if (streamIt == context->StreamStates.end())
					{
						streamIt = context->StreamStates.emplace(serialno, ogg_stream_state()).first;
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
					auto streamIt = context->StreamStates.find(serialno);
					if (streamIt != context->StreamStates.end())
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

void OggMediaSource::SyncBufferFromByteStream(IMFAsyncCallback* asyncCallback, IUnknown* state)
{
	auto buffer = ogg_sync_buffer(&syncState, SizePerReadPeriod);
	ThrowIfNot(buffer, L"Cannot create ogg sync buffer.");
	ThrowIfFailed(byteStream->BeginRead(reinterpret_cast<BYTE*>(buffer), SizePerReadPeriod,
		asyncCallback, state));
}

task<ComPtr<IMFPresentationDescriptor>> OggMediaSource::BuildPresentationDescriptor()
{
	auto context = Make<ReadBOSPacketContext>();
	context->ReadCallback = Make < MFAsyncCallbackWithWeakRef < OggMediaSource,
		MediaSourceBase >> (AsWeak(), &OggMediaSource::OnByteStreamReadCompletedForBOS);
	SyncBufferFromByteStream(context->ReadCallback.Get(), context.Get());
	return create_task(context->ReadCompletionEvent).then([context]
	{
		return MakePresentationDescriptor(context.Get());
	});
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

void OggMediaSource::AddStream(ogg_stream_state && streamState, IMFStreamDescriptor* streamDescriptor)
{
	// 检测 streamDescriptor 的有效性
	DWORD streamId;
	ThrowIfFailed(streamDescriptor->GetStreamIdentifier(&streamId));
	int serialno = streamState.serialno;
	ThrowIfNot(streamId == DWORD(serialno), L"Invalid stream identifier.");
	// 检测没有重复 Id
	ThrowIfNot(mediaStreams.find(serialno) == mediaStreams.end(), L"Conflicted stream identifier.");
	mediaStreams.emplace(serialno, Make<OggDeliverMediaStream>(std::move(streamState), AsWeak(), streamDescriptor));
}