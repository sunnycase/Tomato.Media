//
// Tomato Media Codec
// Media Foundation FFmpeg Audio 解码 Transform
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#include "pch.h"
#include "FFmpegAudioDecoderTransform.h"
#include "../../include/MFRAII.h"
#include "constants.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace NS_MEDIA_CODEC::FFmpeg;
using namespace WRL;
using namespace concurrency;

#if (NTDDI_VERSION >= NTDDI_WIN8)
ActivatableClass(FFmpegAudioDecoderTransform);
#else
//CoCreatableClass(FFmpegAudioDecoderTransform);
#endif

const DecoderTransformRegisterInfo FFmpegAudioDecoderTransform::RegisterInfos[2] =
{
	{ MFAudioFormat_LibAV, MFAudioFormat_Float },
	{ MFAudioFormat_LibAV, MFAudioFormat_PCM },
};

FFmpegAudioDecoderTransform::FFmpegAudioDecoderTransform()
{
}

FFmpegAudioDecoderTransform::~FFmpegAudioDecoderTransform()
{
	if (_opended)
		avcodec_close(_codecContext.get());
}

void FFmpegAudioDecoderTransform::OnValidateInputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Audio) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (subType != MFAudioFormat_LibAV)
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证 Options
	ComPtr<IUnknown> optionsUnk;
	if (FAILED(type->GetUnknown(MF_MT_LIBAV_CODEC_OPTIONS, IID_PPV_ARGS(&optionsUnk))))
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

void FFmpegAudioDecoderTransform::OnValidateOutputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Audio) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (std::find_if(availableOutputTypes.begin(), availableOutputTypes.end(), [&](auto&& myType)
	{
		GUID mySubType;
		ThrowIfFailed(myType->GetGUID(MF_MT_SUBTYPE, &mySubType));
		return mySubType == subType;
	}) == availableOutputTypes.end())
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

DWORD FFmpegAudioDecoderTransform::OnGetOutputFrameSize() const noexcept
{
	return bytesPerDecodecSample * decodedSamples;
}

WRL::ComPtr<IMFMediaType> FFmpegAudioDecoderTransform::OnSetInputType(IMFMediaType * type)
{
	InitializeDecoder(type);
	InitializeAvailableOutputTypes();
	return type;
}

WRL::ComPtr<IMFMediaType> FFmpegAudioDecoderTransform::OnSetOutputType(IMFMediaType * type)
{
	GUID subType;
	UINT32 bitsPerSample;
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &_outputBlockAlign));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &_outputSampleRate));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &_outputChannels));
	bytesPerDecodecSample = _outputBlockAlign;

	_outputSampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
	if (subType == MFAudioFormat_PCM)
	{
		switch (bitsPerSample)
		{
		case 8:
			_outputSampleFormat = AVSampleFormat::AV_SAMPLE_FMT_U8;
			break;
		case 16:
			_outputSampleFormat = AVSampleFormat::AV_SAMPLE_FMT_S16;
			break;
		case 32:
			_outputSampleFormat = AVSampleFormat::AV_SAMPLE_FMT_S32;
			break;
		}
	}
	else if (subType == MFAudioFormat_Float)
	{
		switch (bitsPerSample)
		{
		case 32:
			_outputSampleFormat = AVSampleFormat::AV_SAMPLE_FMT_FLT;
			break;
		case 64:
			_outputSampleFormat = AVSampleFormat::AV_SAMPLE_FMT_DBL;
			break;
		}
	}
	if (_outputSampleFormat == AVSampleFormat::AV_SAMPLE_FMT_NONE)
		ThrowAlways(L"format not support.");

	return type;
}

namespace
{
	void InitPacket(AVPacket& packet, BYTE* data, DWORD length)
	{
		av_init_packet(&packet);
		packet.dts = packet.pts = AV_NOPTS_VALUE;
		packet.data = data;
		packet.size = static_cast<int>(length);
	}
}

bool FFmpegAudioDecoderTransform::OnReceiveInput(IMFSample * sample)
{
	if (FAILED(sample->GetSampleTime(&_sampleTime)))
		_sampleTime = -1;
	ThrowIfFailed(sample->ConvertToContiguousBuffer(&_inputBuffer));
	BYTE* data; DWORD length;
	ThrowIfFailed(_inputBuffer->Lock(&data, nullptr, &length));
	InitPacket(_inputPacket, data, length);
	return DecodeOneFrame();
}

void FFmpegAudioDecoderTransform::OnProduceOutput(MFT_OUTPUT_DATA_BUFFER & output)
{
	if (!output.pSample) ThrowIfFailed(E_INVALIDARG);
	ComPtr<IMFMediaBuffer> buffer;
	ThrowIfFailed(output.pSample->GetBufferByIndex(0, buffer.GetAddressOf()));
	BYTE* data; DWORD maxLength;
	DWORD cntLength = 0;
	{
		Core::MFBufferLocker locker(buffer.Get());
		locker.Lock(data, &maxLength, nullptr);
		if (maxLength < OnGetOutputFrameSize()) ThrowIfFailed(E_INVALIDARG);

		cntLength = FillFrame(buffer.Get(), data, maxLength);
	}
	ThrowIfFailed(buffer->SetCurrentLength(cntLength));
	auto duration = MFTIME(decodedSamples * 1e7 / _outputSampleRate);
	ThrowIfFailed(output.pSample->SetSampleDuration(duration));
	if (_sampleTime != -1)
	{
		ThrowIfFailed(output.pSample->SetSampleTime(_sampleTime));
		_sampleTime += duration;
	}
	if (DecodeOneFrame())
		output.dwStatus = MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
}

bool FFmpegAudioDecoderTransform::DecodeOneFrame()
{
	decodedSamples = 0;
	auto& packet = _inputPacket;
	auto ret = [&] {
		if (packet.size == 0)
			return false;

		int got_frame = 0;
		while (!got_frame)
		{
			auto ret = avcodec_decode_audio4(_codecContext.get(), _frame.get(), &got_frame, &packet);
			if (ret < 0 || (ret == 0 && !got_frame))
				return false;
			packet.size -= ret;
			packet.data += ret;
		}
		decodedSamples = _frame->nb_samples;
		return true;
	}();
	if (!ret && _inputBuffer)
	{
		_inputBuffer->Unlock();
		_inputBuffer.Reset();
	}
	return ret;
}

WRL::ComPtr<IMFMediaType> FFmpegAudioDecoderTransform::OnGetOutputAvailableType(DWORD index) noexcept
{
	if (index < availableOutputTypes.size())
	{
		return availableOutputTypes[index];
	}
	else
		return nullptr;
}

void FFmpegAudioDecoderTransform::InitializeAvailableOutputTypes()
{
	availableOutputTypes.clear();

	auto format = _waveFormat->Format;
	format.wFormatTag = _waveFormat->DesiredDecodedFormat;
	format.cbSize = 0;

	ComPtr<IMFMediaType> outputMediaType;
	ThrowIfFailed(MFCreateMediaType(&outputMediaType));
	ThrowIfFailed(MFInitMediaTypeFromWaveFormatEx(outputMediaType.Get(), &format, sizeof(format)));
	availableOutputTypes.emplace_back(std::move(outputMediaType));
}

void FFmpegAudioDecoderTransform::InitializeDecoder(IMFMediaType* inputType)
{
	WAVEFORMATEX* tmpFormat = nullptr; UINT32 size;
	ThrowIfFailed(MFCreateWaveFormatExFromMFMediaType(inputType, &tmpFormat, &size));
	_waveFormat.reset(reinterpret_cast<WAVEFORMATLIBAV*>(tmpFormat));
	auto codec = avcodec_find_decoder(_waveFormat->CodecId);
	ThrowIfNot(codec, L"Cannot find a codec.");
	_codecContext.reset(avcodec_alloc_context3(codec));
	ThrowIfNot(_codecContext, L"Cannot allocate codec context.");
	_codecContext->sample_fmt = _waveFormat->SampleFormat;
	_codecContext->sample_rate = (int)tmpFormat->nSamplesPerSec;
	_codecContext->channels = (int)tmpFormat->nChannels;
	_codecContext->block_align = (int)tmpFormat->nBlockAlign;
	_codecContext->bits_per_coded_sample = _waveFormat->BitsPerCodedSample;
	_codecContext->flags = _waveFormat->Flags;
	_codecContext->flags2 = _waveFormat->Flags2;

	ComPtr<IUnknown> optionsUnk;
	ThrowIfFailed(inputType->GetUnknown(MF_MT_LIBAV_CODEC_OPTIONS, IID_PPV_ARGS(&optionsUnk)));
	auto options = static_cast<LibAVCodecOptions*>(optionsUnk.Get());
	if (!options->ExtraData.empty())
	{
		auto size = options->ExtraData.size();
		_codecContext->extradata = reinterpret_cast<uint8_t*>(av_malloc(size));
		ThrowIfNot(memcpy_s(_codecContext->extradata, size, options->ExtraData.data(), size) == 0, L"Cannot copy extra data");
		_codecContext->extradata_size = (int)size;
	}
	ThrowIfNot(avcodec_open2(_codecContext.get(), codec, nullptr) >= 0, L"Cannot open codec context.");
	_opended = true;

	_frame.reset(av_frame_alloc());
	ThrowIfNot(_frame, L"Cannot allocate frame.");
}

DWORD FFmpegAudioDecoderTransform::FillFrame(IMFMediaBuffer* buffer, BYTE * data, DWORD maxLength)
{
	auto swrContext = swr_alloc_set_opts(_swrContext.get(), av_get_default_channel_layout(_outputChannels),
		_outputSampleFormat, _outputSampleRate, _frame->channel_layout, _waveFormat->SampleFormat,
		_waveFormat->Format.nSamplesPerSec, 0, nullptr);
	if (swrContext != _swrContext.get())
	{
		_swrContext.reset(swrContext);
		ThrowIfNot(swrContext && swr_init(swrContext) >= 0, L"Cannot load resampler.");
	}
	ThrowIfNot(swrContext, L"Cannot load resampler.");

	int converted = 0;
	uint8_t* out[] = { data };
	converted = swr_convert(swrContext, out, _frame->nb_samples, (const uint8_t**)_frame->extended_data, _frame->nb_samples);
	ThrowIfNot(converted == _frame->nb_samples, L"Resample failed.");
	av_frame_unref(_frame.get());
	return converted * _outputBlockAlign;
}

void FFmpegAudioDecoderTransform::BeginStreaming()
{
}