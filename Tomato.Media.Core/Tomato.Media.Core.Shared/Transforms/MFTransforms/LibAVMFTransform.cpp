//
// Tomato Media
// LibAV Media Foundation Transform
// 
// (c) SunnyCase 
// 创建日期 2015-03-16
#include "pch.h"
#include "LibAVMFTransform.h"
#include "Utilities/mfhelpers.hpp"
#include "Utilities/libavhelpers.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

ActivatableClass(LibAVMFTransform);

LibAVMFTransform::LibAVMFTransform()
{
	RegisterLibAV();
}

LibAVMFTransform::~LibAVMFTransform()
{
	if (codecOpened)
	{
		avcodec_close(waveFormat.CodecContext);
		codecOpened = false;
	}
	if (swrContext)
		swr_free(&swrContext);
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
	return outputBufferSize;
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
	codecOpened = true;

	WAVEFORMATEX* format;
	UINT32 size;
	THROW_IF_FAILED(MFCreateWaveFormatExFromMFMediaType(type, &format, &size));
	unique_cotaskmem<WAVEFORMATEX> coFormat(format);
	outputFormat = *format;

	sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
	if (format->wFormatTag == WAVE_FORMAT_PCM)
	{
		switch (format->wBitsPerSample)
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
	else if (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
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

	swrContext = swr_alloc_set_opts(nullptr, av_get_default_channel_layout(
		outputFormat.nChannels), sampleFormat, outputFormat.nSamplesPerSec,
		waveFormat.CodecContext->channel_layout,
		waveFormat.CodecContext->sample_fmt, waveFormat.CodecContext->sample_rate, 0, nullptr);
	THROW_IF_NOT(swrContext && swr_init(swrContext) >= 0, "Cannot load resampler.");
	return type;
}

void LibAVMFTransform::OnReceiveInput(IMFSample * sample)
{
	DWORD bufferCount;
	ComPtr<IMFMediaBuffer> buffer;
	THROW_IF_FAILED(sample->GetBufferCount(&bufferCount));
	THROW_IF_NOT(bufferCount == 1, "Invalid buffer count.");
	THROW_IF_FAILED(sample->GetBufferByIndex(0, &buffer));

	ComPtr<IMFSample> outputSample;
	THROW_IF_FAILED(MFCreateSample(&outputSample));

	outputSamples = 0;
	outputBufferSize = 0;
	{
		mfbuffer_locker locker(buffer.Get());
		BYTE* data; DWORD bufferLen;
		THROW_IF_FAILED(locker.lock(&data, nullptr, &bufferLen));
		AVPacket packet;
		av_init_packet(&packet);
		packet.data = data;
		packet.size = bufferLen;
		packet.dts = packet.pts = AV_NOPTS_VALUE;

		while (true)
		{
			ComPtr<IMFMediaBuffer> buffer;
			auto pair = DecodeFrame(packet, buffer);
			if (buffer)
				THROW_IF_FAILED(outputSample->AddBuffer(buffer.Get()));
			outputSamples += pair.first;
			if (!pair.second) break;
		}
	}
	THROW_IF_FAILED(outputSample->ConvertToContiguousBuffer(outputBuffer.ReleaseAndGetAddressOf()));
}

void LibAVMFTransform::OnProduceOutput(IMFSample * input, MFT_OUTPUT_DATA_BUFFER & output)
{
	if (!output.pSample)
		THROW_IF_FAILED(E_POINTER);
	DWORD bufferCount;
	ComPtr<IMFMediaBuffer> buffer;
	THROW_IF_FAILED(output.pSample->GetBufferCount(&bufferCount));
	THROW_IF_NOT(bufferCount == 1, "Invalid buffer count.");
	THROW_IF_FAILED(output.pSample->GetBufferByIndex(0, &buffer));

	{
		mfbuffer_locker lockerIn(outputBuffer.Get()), lockerOut(buffer.Get());
		BYTE* dataIn, *dataOut;
		DWORD maxLengthIn, maxLengthOut, cntLengthIn, cntLengthOut;
		THROW_IF_FAILED(lockerIn.lock(&dataIn, &maxLengthIn, &cntLengthIn));
		THROW_IF_FAILED(lockerOut.lock(&dataOut, &maxLengthOut, &cntLengthOut));
		THROW_IF_NOT(maxLengthOut >= cntLengthIn, "Buffer is too small.");
		THROW_IF_NOT(memcpy_s(dataOut, maxLengthOut, dataIn, cntLengthIn) == 0,
			"Copy data failed.");
	}

	THROW_IF_FAILED(buffer->SetCurrentLength(outputBufferSize));

	LONGLONG sampleTime = 0;
	if (SUCCEEDED(input->GetSampleTime(&sampleTime)))
		THROW_IF_FAILED(output.pSample->SetSampleTime(sampleTime));

	THROW_IF_FAILED(output.pSample->SetSampleDuration(static_cast<LONGLONG>(
		outputSamples * 1.0e7 / outputFormat.nSamplesPerSec)));

	//output.pSample = outputSample.Detach();
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

	if (!frame)
	{
		frame.reset(av_frame_alloc());
		THROW_IF_NOT(frame, "Cannot allocate frame.");
	}

	int got_frame = 0;

	auto ret = avcodec_decode_audio4(waveFormat.CodecContext, frame.get(), &got_frame, &packet);
	if (ret < 0)
		return{ 0, false };

	packet.data += ret;
	packet.size -= ret;

	if (got_frame)
	{
		auto size = av_samples_get_buffer_size(nullptr, outputFormat.nChannels,
			frame->nb_samples, sampleFormat, outputFormat.nBlockAlign);

		THROW_IF_FAILED(MFCreateMemoryBuffer(size, buffer.ReleaseAndGetAddressOf()));

		int converted = 0;
		{
			mfbuffer_locker locker(buffer.Get());
			BYTE* data;
			THROW_IF_FAILED(locker.lock(&data, nullptr, nullptr));
			uint8_t* out[] = { data };
			converted = swr_convert(swrContext, out, frame->nb_samples,
				(const uint8_t**)frame->extended_data, frame->nb_samples);
			THROW_IF_NOT(converted == frame->nb_samples, "Resample failed.");
		}

		av_frame_unref(frame.get());
		THROW_IF_FAILED(buffer->SetCurrentLength(size));

		outputBufferSize += size;
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
