//
// Tomato Media Codec
// Media Foundation FFmpeg MediaSource
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#include "pch.h"
#include "FFmpegMediaSource.h"
#include "constants.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace NS_MEDIA_CODEC::FFmpeg;
using namespace WRL;
using namespace concurrency;

enum : long
{
	SizePerReadPeriod = 64 * 1024	// 64 KB
};

#define LOCK_STATE() std::lock_guard<decltype(stateMutex)> locker(stateMutex)

namespace
{
	MFOffset MakeOffset(float v)
	{
		MFOffset offset;
		offset.value = short(v);
		offset.fract = WORD(65536 * (v - offset.value));
		return offset;
	}

	//struct VideoStreamDetector
	//{
	//	static bool IsMyHeader(AVStream* stream)
	//	{
	//		return stream->codec->codec_type == AVMEDIA_TYPE_VIDEO;
	//	}

	//	static ComPtr<IMFStreamDescriptor> BuildStreamDescriptor(const ogg_stream_state& streamState)
	//	{
	//		auto myStreamState = streamState;

	//		th_info info;
	//		th_info_init(&info);

	//		ogg_packet packet;
	//		if (ogg_stream_packetout(&myStreamState, &packet) != 1) ThrowIfFailed(MF_E_INVALID_STREAM_DATA);
	//		if (th_decode_headerin(&info, nullptr, nullptr, &packet) < 0) ThrowIfFailed(MF_E_INVALID_STREAM_DATA);

	//		MFVideoArea area;
	//		area.OffsetX = MakeOffset(info.pic_x);
	//		area.OffsetY = MakeOffset(info.pic_y);
	//		area.Area.cx = info.pic_width;
	//		area.Area.cy = info.pic_height;

	//		ComPtr<IMFMediaType> mediaType;
	//		ThrowIfFailed(MFCreateMediaType(&mediaType));
	//		ThrowIfFailed(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
	//		ThrowIfFailed(mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_Theora));
	//		ThrowIfFailed(MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, info.frame_width, info.frame_height));
	//		ThrowIfFailed(MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, info.fps_numerator, info.fps_denominator));
	//		ThrowIfFailed(mediaType->SetUINT32(MF_MT_THEORA_PIXEL_FORMAT, info.pixel_fmt));
	//		ThrowIfFailed(mediaType->SetUINT32(MF_MT_PAN_SCAN_ENABLED, TRUE));
	//		ThrowIfFailed(MFSetAttributeRatio(mediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, info.aspect_numerator, info.aspect_denominator));
	//		ThrowIfFailed(mediaType->SetBlob(MF_MT_PAN_SCAN_APERTURE, reinterpret_cast<
	//			const UINT8*>(&area), sizeof(area)));

	//		ComPtr<IMFStreamDescriptor> desc;
	//		ThrowIfFailed(MFCreateStreamDescriptor(DWORD(streamState.serialno), 1,
	//			mediaType.GetAddressOf(), &desc));
	//		ComPtr<IMFMediaTypeHandler> mediaTypeHandler;
	//		ThrowIfFailed(desc->GetMediaTypeHandler(&mediaTypeHandler));
	//		ThrowIfFailed(mediaTypeHandler->SetCurrentMediaType(mediaType.Get()));

	//		return desc;
	//	}
	//};

	struct AudioStreamDetector
	{
		static bool IsMyHeader(AVStream* stream)
		{
			return stream->codec->codec_type == AVMEDIA_TYPE_AUDIO;
		}

		static ComPtr<IMFStreamDescriptor> BuildStreamDescriptor(AVStream* stream)
		{
			auto waveFormat(WAVEFORMATLIBAV::CreateFromStream(stream));
			ComPtr<IMFMediaType> mediaType;
			ThrowIfFailed(MFCreateMediaType(&mediaType));
			ThrowIfFailed(MFInitMediaTypeFromWaveFormatEx(mediaType.Get(), &waveFormat.Format, sizeof(waveFormat)));

			ComPtr<IMFStreamDescriptor> desc;
			ThrowIfFailed(MFCreateStreamDescriptor(DWORD(stream->id), 1, mediaType.GetAddressOf(), &desc));
			ComPtr<IMFMediaTypeHandler> mediaTypeHandler;
			ThrowIfFailed(desc->GetMediaTypeHandler(&mediaTypeHandler));
			ThrowIfFailed(mediaTypeHandler->SetCurrentMediaType(mediaType.Get()));

			return desc;
		}
	};

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

	ComPtr<IMFPresentationDescriptor> MakePresentationDescriptor(AVFormatContext* fmtCtx)
	{
		std::vector<ComPtr<IMFStreamDescriptor>> streamDescriptors;
		auto lastStreamIt = fmtCtx->streams + fmtCtx->nb_streams;
		for (auto it = fmtCtx->streams;it != lastStreamIt;++it)
		{
			auto stream = *it;
			ComPtr<IMFStreamDescriptor> streamDescriptor;

			if (AudioStreamDetector::IsMyHeader(stream))
				streamDescriptors.emplace_back(AudioStreamDetector::BuildStreamDescriptor(stream));
			//else if (VorbisStreamDetector::IsMyHeader(streamState))
			//	streamDescriptor = VorbisStreamDetector::BuildStreamDescriptor(streamState);
		}
		// 没有一个可识别的流，则引发异常
		if (streamDescriptors.empty())
			ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
		ComPtr<IMFPresentationDescriptor> presentationDescriptor;
		ThrowIfFailed(MFCreatePresentationDescriptor(streamDescriptors.size(), reinterpret_cast<
			IMFStreamDescriptor**>(streamDescriptors.data()), &presentationDescriptor));
		// 设置 Context 属性
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

FFmpegMediaSource::FFmpegMediaSource()
	:workerQueue(MFASYNC_CALLBACK_QUEUE_UNDEFINED)
{
	FFmpeg::Initialize();
}

FFmpegMediaSource::~FFmpegMediaSource()
{

}

HRESULT FFmpegMediaSource::GetService(REFGUID guidService, REFIID riid, LPVOID * ppvObject)
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

DWORD FFmpegMediaSource::OnGetCharacteristics()
{
	return MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_CAN_SEEK;
}

void FFmpegMediaSource::OnValidatePresentationDescriptor(IMFPresentationDescriptor * pPD)
{
	DWORD streamCount;
	ThrowIfFailed(pPD->GetStreamDescriptorCount(&streamCount));
	const auto streams = (*_fmtContext)->streams;
	const auto lastStreamIt = streams + (*_fmtContext)->nb_streams;
	for (DWORD i = 0; i < streamCount; i++)
	{
		BOOL selected;
		ComPtr<IMFStreamDescriptor> streamDescriptor;
		ThrowIfFailed(pPD->GetStreamDescriptorByIndex(i, &selected, &streamDescriptor));

		// 检测 streamDescriptor 的有效性
		DWORD streamId;
		ThrowIfFailed(streamDescriptor->GetStreamIdentifier(&streamId));
		// 添加流
		if (selected)
		{
			auto streamStateIt = std::find_if(streams, lastStreamIt, [id = (int)streamId](AVStream* stream){ return stream->id == id; });
			if (streamStateIt != lastStreamIt)
				AddStream(*streamStateIt, streamDescriptor.Get());
		}
	}
}

concurrency::task<WRL::ComPtr<IMFPresentationDescriptor>> FFmpegMediaSource::OnCreatePresentationDescriptor(IMFByteStream * stream)
{
	RegisterWorkThreadIfNeeded();

	return BuildPresentationDescriptor(stream);
}

concurrency::task<void> FFmpegMediaSource::OnStreamsRequestData(IMFMediaStream* mediaStream)
{
	{
		LOCK_STATE();
		if (state != MFMediaSourceState::Started)
			return task_from_result();
	}

	auto deliverStream = static_cast<FFmpegDeliverMediaStream*>(mediaStream);
	if (deliverStream->DoesNeedMoreData())
	{
		auto desiredStreamIdx = deliverStream->GetIndex();
		auto fmtctx = _fmtContext->Get();

		AVPacket packet{ 0 };
		av_init_packet(&packet);

		bool got = false;
		while (!got)
		{
			auto ret = av_read_frame(fmtctx, &packet);
			try
			{
				if (ret < 0)
				{
					if ((ret == AVERROR_EOF || avio_feof(fmtctx->pb)) ||
						(fmtctx->pb && fmtctx->pb->error))
					{
						deliverStream->EndOfDeliver();
					}
					return task_from_result();
				}
				else
				{
					auto streamIt = _streamIndexMaps.find(packet.stream_index);
					if (streamIt != _streamIndexMaps.end())
					{
						auto toDeliverStream = streamIt->second.Get();
						toDeliverStream->DeliverPacket(packet);
						if(packet.stream_index == desiredStreamIdx)
							got = true;
					}
				}
				av_free_packet(&packet);
			}
			catch (...)
			{
				av_free_packet(&packet);
				throw;
			}
		}
	}
	return task_from_result();
}

void FFmpegMediaSource::OnSeekSource(MFTIME position)
{
	QWORD offset;
	ThrowIfNot(av_seek_frame(_fmtContext->Get(), -1, hns2dt(position, nullptr), AVSEEK_FLAG_ANY) >= 0, L"Cannot seek.");
}

void FFmpegMediaSource::OnStartStream(DWORD streamId, bool selected, const PROPVARIANT& position)
{
	auto streamIt = _streamIdMaps.find(int(streamId));
	if (streamIt == _streamIdMaps.end())
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

void FFmpegMediaSource::OnPauseStream(DWORD streamId)
{
	auto streamIt = _streamIdMaps.find(int(streamId));
	if (streamIt == _streamIdMaps.end())
		ThrowIfFailed(E_INVALIDARG);
	streamIt->second->Pause();
}

void FFmpegMediaSource::OnStopStream(DWORD streamId)
{
	auto streamIt = _streamIdMaps.find(int(streamId));
	if (streamIt == _streamIdMaps.end())
		ThrowIfFailed(E_INVALIDARG);
	streamIt->second->Stop();
}

task<ComPtr<IMFPresentationDescriptor>> FFmpegMediaSource::BuildPresentationDescriptor(IMFByteStream * stream)
{
	auto ioContext = std::make_shared<MFAVIOContext>(stream, 4096, false);
	auto fmtContext(ioContext->OpenFormatContext());
	ThrowIfNot(avformat_find_stream_info(fmtContext.Get(), nullptr) >= 0, L"Read stream info error.");
#if _DEBUG
	av_dump_format(fmtContext.Get(), 0, nullptr, 0);
#endif
	_fmtContext = std::make_shared<FFmpeg::Wrappers::AVFormatContextWrapper>(std::move(fmtContext));
	return task_from_result(MakePresentationDescriptor(_fmtContext->Get()));
}

void FFmpegMediaSource::RegisterWorkThreadIfNeeded()
{
	if (workThreadRegistered) return;

	if (!workerQueue.IsValid())
		workerQueue = MFWorkerQueueProvider::GetAudio();

	workThreadRegistered = true;
}

void FFmpegMediaSource::EndOfSource()
{

}

void FFmpegMediaSource::AddStream(AVStream* stream, IMFStreamDescriptor* streamDescriptor)
{
	auto deliverStream = Make<FFmpegDeliverMediaStream>(stream, AsWeak(), streamDescriptor);
	_streamIdMaps.emplace(stream->id, deliverStream);
	_streamIndexMaps.emplace(stream->index, deliverStream);
}