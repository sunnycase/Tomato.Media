//
// Tomato Media
// Media Foundation LibAV Audio Source
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#include "pch.h"
#include "LibAVAudioSource.h"
#include "Utilities/mfhelpers.hpp"
#include "Utilities/libavhelpers.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

inline REFERENCE_TIME dt2hns(int dt, AVStream* stream)
{
	auto sec_base = (float)stream->time_base.num / stream->time_base.den;
	auto hns = dt * sec_base * 1e7;
	return static_cast<REFERENCE_TIME>(hns);
}

inline int hns2dt(REFERENCE_TIME hns, AVStream* stream)
{
	auto sec_base = (float)stream->time_base.num / stream->time_base.den;
	auto dt = hns / sec_base / 1e7;
	return static_cast<int>(dt);
}

LibAVAudioSource::LibAVAudioSource()
{
	RegisterLibAV();
}

LibAVAudioSource::~LibAVAudioSource()
{

}

DWORD LibAVAudioSource::OnGetCharacteristics()
{
	return MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_CAN_SEEK;
}

task<std::vector<ComPtr<IMFMediaType>>> LibAVAudioSource::OnCreateMediaTypes(ComPtr<IMFByteStream> stream)
{
	CreateAVFormatContext(stream);
	std::vector<ComPtr<IMFMediaType>> mediaTypes = { CreateMediaType() };
	return task_from_result(mediaTypes);
}

ComPtr<IMFMediaType> LibAVAudioSource::CreateMediaType()
{
	auto codecContext = audioStream->codec;

	ComPtr<IMFMediaType> mediaType;
	THROW_IF_FAILED(MFCreateMediaType(&mediaType));

	auto waveFormat = WAVEFORMATLIBAV::CreateFromFormatContext(avfmtctx, audioStream->index);

	THROW_IF_FAILED(MFInitMediaTypeFromWaveFormatEx(mediaType.Get(), &waveFormat.Format, sizeof(waveFormat)));
	return mediaType;
}

void LibAVAudioSource::OnStartAudioStream(REFERENCE_TIME position)
{
	THROW_IF_NOT(av_seek_frame(avfmtctx.get(), audioStream->index,
		hns2dt(position, audioStream), AVSEEK_FLAG_ANY) >= 0, "Seek failed");
}

void LibAVAudioSource::SeekToFrame(uint32_t frameId)
{
	THROW_IF_NOT(av_seek_frame(avfmtctx.get(), audioStream->index, frameId, AVSEEK_FLAG_FRAME) >= 0,
		"Seek failed.");
}

task<bool> LibAVAudioSource::OnReadSample(ComPtr<IMFSample> sample)
{
	auto fmtctx = avfmtctx.get();

	AVPacket packet{ 0 };
	av_init_packet(&packet);

	bool got = false;
	while (!got)
	{
		auto ret = av_read_frame(fmtctx, &packet);
		if (ret < 0)
		{
			if ((ret == AVERROR_EOF || avio_feof(fmtctx->pb)) ||
				(fmtctx->pb && fmtctx->pb->error))
			{
				EndOfDeliver();
			}
			return task_from_result(false);
		}
		else if (packet.buf)
		{
			if (packet.stream_index == audioStream->index)
			{
				auto size = packet.size;
				ComPtr<IMFMediaBuffer> mediaBuffer;
				auto maxSize = packet.size - packet.size % FF_INPUT_BUFFER_PADDING_SIZE;
				if (maxSize < packet.size) maxSize += FF_INPUT_BUFFER_PADDING_SIZE;
				THROW_IF_FAILED(MFCreateMemoryBuffer(maxSize, &mediaBuffer));
				{
					mfbuffer_locker locker(mediaBuffer.Get());
					BYTE* data;
					THROW_IF_FAILED(locker.lock(&data, nullptr, nullptr));
					memset(data, 0, maxSize);
					memcpy_s(data, size, packet.data, packet.size);
				}
				THROW_IF_FAILED(mediaBuffer->SetCurrentLength(packet.size));
				THROW_IF_FAILED(sample->AddBuffer(mediaBuffer.Get()));
				THROW_IF_FAILED(sample->SetSampleTime(dt2hns(packet.pts, audioStream) / 10));
				got = true;
			}
		}
		av_free_packet(&packet);
	}
	return task_from_result(true);
}

void LibAVAudioSource::OnConfigurePresentationDescriptor(IMFPresentationDescriptor *pPD)
{
	THROW_IF_FAILED(pPD->SetUINT64(MF_PD_DURATION, dt2hns(audioStream->duration, audioStream)));
}

void LibAVAudioSource::CreateAVFormatContext(wrl::ComPtr<IMFByteStream> stream)
{
	avioctx = std::make_unique<MFAVIOContext>(stream, 4096 * 10, false);

	auto fmtctx = OpenAVFormatContext(avioctx->Get());
	THROW_IF_NOT(avformat_find_stream_info(fmtctx.get(), nullptr) >= 0, L"Read stream info error.");
#if _DEBUG
	av_dump_format(fmtctx.get(), 0, nullptr, 0);
#endif
	auto lastStreamIt = fmtctx->streams + fmtctx->nb_streams;
	// 找到第一个音频流
	auto audioStreamIt = std::find_if(fmtctx->streams, lastStreamIt,
		[](AVStream* stream) {return stream->codec->codec_type == AVMEDIA_TYPE_AUDIO; });
	THROW_IF_NOT(audioStreamIt != lastStreamIt, "Cannot find a audio stream.");

	avfmtctx = std::move(fmtctx);
	audioStream = *audioStreamIt;
}
