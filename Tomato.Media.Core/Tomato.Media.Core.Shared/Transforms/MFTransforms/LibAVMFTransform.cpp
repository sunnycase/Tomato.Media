//
// Tomato Media
// LibAV Media Foundation Transform
// 
// (c) SunnyCase 
// 创建日期 2015-03-16
#include "pch.h"
#include "LibAVMFTransform.h"
#include "Utilities/mfhelpers.hpp"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

ActivatableClass(LibAVMFTransform);

LibAVMFTransform::LibAVMFTransform()
{
}

LibAVMFTransform::~LibAVMFTransform()
{

}

void LibAVMFTransform::OnValidateInputType(IMFMediaType * type)
{
	static auto allowSubtypes = {
		KSDATAFORMAT_SUBTYPE_LIBAV
	};

	GUID majorType, subType;

	// 验证主类型
	THROW_IF_FAILED(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Audio) THROW_IF_FAILED(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	THROW_IF_FAILED(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (std::find(allowSubtypes.begin(), allowSubtypes.end(), subType) == allowSubtypes.end())
		THROW_IF_FAILED(MF_E_INVALIDMEDIATYPE);
}

void LibAVMFTransform::OnValidateOutputType(IMFMediaType * type)
{
	static auto allowSubtypes = {
		MFAudioFormat_PCM,
		MFAudioFormat_Float
	};

	GUID majorType, subType;

	// 验证主类型
	THROW_IF_FAILED(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Audio) THROW_IF_FAILED(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	THROW_IF_FAILED(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (std::find(allowSubtypes.begin(), allowSubtypes.end(), subType) == allowSubtypes.end())
		THROW_IF_FAILED(MF_E_INVALIDMEDIATYPE);
}

DWORD LibAVMFTransform::OnGetOutputFrameSize() const noexcept
{
	return 0;
}

HRESULT LibAVMFTransform::GetOutputStreamInfo(
	DWORD                     dwOutputStreamID,
	MFT_OUTPUT_STREAM_INFO *  pStreamInfo
	)
{
	auto hr = AudioFrameDecoderBase::GetOutputStreamInfo(dwOutputStreamID, pStreamInfo);
	if (SUCCEEDED(hr))
		pStreamInfo->dwFlags |= MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;

	return hr;
}

ComPtr<IMFMediaType> LibAVMFTransform::OnSetInputType(IMFMediaType * type)
{
	InitializeLibAVFormat(type);
	InitializeAvailableOutputTypes();
	return type;
}

ComPtr<IMFMediaType> LibAVMFTransform::OnSetOutputType(IMFMediaType * type)
{
	auto codec = avcodec_find_decoder(waveFormat.CodecContext->codec_id);
	THROW_IF_NOT(codec, "Cannot find decoder.");
	THROW_IF_NOT(avcodec_open2(waveFormat.CodecContext, codec, nullptr) == 0,
		"Open decoder error.");

	WAVEFORMATEX* format;
	UINT32 size;
	THROW_IF_FAILED(MFCreateWaveFormatExFromMFMediaType(type, &format, &size));
	unique_cotaskmem<WAVEFORMATEX> coFormat(format);
	outputFormat = *format;
	return type;
}

void LibAVMFTransform::OnReceiveInput(IMFSample * sample)
{
}

void LibAVMFTransform::OnProduceOutput(IMFSample * input, MFT_OUTPUT_DATA_BUFFER & output)
{
	ComPtr<IMFSample> outputSample;
	THROW_IF_FAILED(MFCreateSample(&outputSample));
	LONGLONG sampleTime = 0;

	ComPtr<IMFMediaBuffer> buffer;
	THROW_IF_FAILED(input->GetBufferByIndex(0, &buffer));
	DWORD bufferLen = 0;
	THROW_IF_FAILED(buffer->GetCurrentLength(&bufferLen));

	mfbuffer_locker locker(buffer.Get());
	BYTE* data;
	THROW_IF_FAILED(locker.lock(&data, nullptr, nullptr));
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = data;
	packet.size = bufferLen;
	packet.dts = packet.pts = AV_NOPTS_VALUE;

	size_t samples = 0;
	while (true)
	{
		ComPtr<IMFMediaBuffer> buffer;
		auto pair = DecodeFrame(packet, buffer);
		if (buffer)
			THROW_IF_FAILED(outputSample->AddBuffer(buffer.Get()));
		samples += pair.first;
		if (!pair.second) break;
	}

	THROW_IF_FAILED(outputSample->SetSampleDuration(samples * 1.0e7 /
		(double)outputFormat.nSamplesPerSec));

	output.pSample = outputSample.Detach();
}

void LibAVMFTransform::InitializeLibAVFormat(IMFMediaType* type)
{
	WAVEFORMATEX* pFormat = nullptr;
	UINT32 cbSize;
	THROW_IF_FAILED(MFCreateWaveFormatExFromMFMediaType(type, &pFormat, &cbSize));
	unique_cotaskmem<WAVEFORMATEX> coFormat(pFormat);

	waveFormat = *reinterpret_cast<WAVEFORMATLIBAV*>(pFormat);
}

void LibAVMFTransform::InitializeAvailableOutputTypes()
{
	availableOutputTypes.clear();

	auto decodedFormat = waveFormat.Format;
	decodedFormat.cbSize = 0;
	decodedFormat.wFormatTag = WAVE_FORMAT_PCM;

	ComPtr<IMFMediaType> type;
	THROW_IF_FAILED(MFCreateMediaType(&type));
	THROW_IF_FAILED(MFInitMediaTypeFromWaveFormatEx(type.Get(), &decodedFormat, sizeof(decodedFormat)));
	availableOutputTypes.emplace_back(std::move(type));

	decodedFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	THROW_IF_FAILED(MFCreateMediaType(&type));
	THROW_IF_FAILED(MFInitMediaTypeFromWaveFormatEx(type.Get(), &decodedFormat, sizeof(decodedFormat)));
	availableOutputTypes.emplace_back(std::move(type));
}

std::pair<uint32_t, bool> LibAVMFTransform::DecodeFrame(AVPacket& packet, wrl::ComPtr<IMFMediaBuffer>& buffer)
{
	if (packet.size == 0)
		return{ 0, false };

	auto frame = av_frame_alloc();
	int got_frame = 0;

	auto ret = avcodec_decode_audio4(waveFormat.CodecContext, frame, &got_frame, &packet);
	if (ret < 0)
		return{ 0, false };

	packet.data += ret;
	packet.size -= ret;

	if (got_frame)
	{
		auto& format = outputFormat;
		AVSampleFormat sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
		if (format.wFormatTag == WAVE_FORMAT_PCM)
		{
			switch (format.wBitsPerSample)
			{
			case 8:
				sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_U8;
				break;
			case 16:
				sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_S16;
				break;
			case 32:
				sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_S32;
				break;
			}
		}
		else if (format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
		{
			switch (waveFormat.Format.wBitsPerSample)
			{
			case 16:
				sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_FLT;
				break;
			case 32:
				sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_DBL;
				break;
			}
		}
		if (sampleFormat == AVSampleFormat::AV_SAMPLE_FMT_NONE)
			THROW_IF_NOT(false, "format not support.");

		auto size = av_samples_get_buffer_size(nullptr, format.nChannels,
			frame->nb_samples, sampleFormat, format.nBlockAlign);

		THROW_IF_FAILED(MFCreateMemoryBuffer(size, buffer.ReleaseAndGetAddressOf()));
		auto swrCtx = swr_alloc_set_opts(nullptr, av_get_default_channel_layout(format.nChannels),
			sampleFormat, format.nSamplesPerSec, frame->channel_layout,
			waveFormat.CodecContext->sample_fmt, waveFormat.CodecContext->sample_rate, 0, nullptr);
		int converted = 0;
		THROW_IF_NOT(swrCtx && swr_init(swrCtx) >= 0, "Cannot load resampler.");
		{
			mfbuffer_locker locker(buffer.Get());
			BYTE* data;
			THROW_IF_FAILED(locker.lock(&data, nullptr, nullptr));
			uint8_t* out[] = { data };
			converted = swr_convert(swrCtx, out, frame->nb_samples,
				(const uint8_t**)frame->extended_data, frame->nb_samples);
			THROW_IF_NOT(converted == frame->nb_samples, "Resample failed.");
		}
		av_frame_unref(frame);
		av_frame_free(&frame);
		swr_free(&swrCtx);

		THROW_IF_FAILED(buffer->SetCurrentLength(size));

		return{ converted, packet.size > 0 };
	}
	else
		return{ 0, true };
}

ComPtr<IMFMediaType> LibAVMFTransform::OnGetOutputAvailableType(DWORD index) noexcept
{
	if (index < availableOutputTypes.size())
	{
		return availableOutputTypes[index];
	}
	else
		return nullptr;
}
