//
// Tomato Media Codec
// Media Foundation FFmpeg Video 解码 Transform
// 
// 作者：SunnyCase
// 创建时间：2016-05-18
#include "pch.h"
#include "FFmpegVideoDecoderTransform.h"
#include "../../include/MFRAII.h"
#include "constants.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace NS_MEDIA_CODEC::FFmpeg;
using namespace WRL;
using namespace concurrency;

#if (NTDDI_VERSION >= NTDDI_WIN8)
ActivatableClass(FFmpegVideoDecoderTransform);
#else
//CoCreatableClass(FFmpegVideoDecoderTransform);
#endif

const DecoderTransformRegisterInfo FFmpegVideoDecoderTransform::RegisterInfos[1] =
{
	{ MFVideoFormat_LibAV, MFVideoFormat_YV12 }
};

FFmpegVideoDecoderTransform::FFmpegVideoDecoderTransform()
{
}

FFmpegVideoDecoderTransform::~FFmpegVideoDecoderTransform()
{
	if (_opended)
		avcodec_close(_codecContext.get());
}

void FFmpegVideoDecoderTransform::OnValidateInputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Video) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (subType != MFVideoFormat_LibAV)
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证 Options
	ComPtr<IUnknown> optionsUnk;
	if (FAILED(type->GetUnknown(MF_MT_LIBAV_VIDEO_CODEC_OPTIONS, IID_PPV_ARGS(&optionsUnk))))
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

void FFmpegVideoDecoderTransform::OnValidateOutputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Video) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
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

DWORD FFmpegVideoDecoderTransform::OnGetOutputFrameSize() const noexcept
{
	return avpicture_get_size(_outputFormat, _width, _height);
}

WRL::ComPtr<IMFMediaType> FFmpegVideoDecoderTransform::OnSetInputType(IMFMediaType * type)
{
	InitializeDecoder(type);
	InitializeAvailableOutputTypes(type);
	return type;
}

WRL::ComPtr<IMFMediaType> FFmpegVideoDecoderTransform::OnSetOutputType(IMFMediaType * type)
{
	GUID subType;
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	ThrowIfFailed(MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &_width, &_height));
	ThrowIfFailed(type->GetUINT64(MF_MT_FRAME_RATE, reinterpret_cast<UINT64*>(&_framerate)));
	ThrowIfFailed(type->GetUINT64(MF_MT_PIXEL_ASPECT_RATIO, reinterpret_cast<UINT64*>(&_aspectRatio)));

	if (subType == MFVideoFormat_YV12)
	{
		_outputFormat = AVPixelFormat::PIX_FMT_YUV420P;
	}
	else
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

bool FFmpegVideoDecoderTransform::OnReceiveInput(IMFSample * sample)
{
	if (FAILED(sample->GetSampleTime(&_sampleTime)))
		_sampleTime = -1;
	ThrowIfFailed(sample->ConvertToContiguousBuffer(&_inputBuffer));
	BYTE* data; DWORD length;
	ThrowIfFailed(_inputBuffer->Lock(&data, nullptr, &length));
	InitPacket(_inputPacket, data, length);
	return DecodeOneFrame();
}

void FFmpegVideoDecoderTransform::OnProduceOutput(MFT_OUTPUT_DATA_BUFFER & output)
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
	auto duration = MFTIME(_framerate.Numerator * 1e7 / _framerate.Denominator);
	ThrowIfFailed(output.pSample->SetSampleDuration(duration));
	if (_sampleTime != -1)
	{
		ThrowIfFailed(output.pSample->SetSampleTime(_sampleTime));
		_sampleTime += duration;
	}
	if (DecodeOneFrame())
		output.dwStatus = MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
}

bool FFmpegVideoDecoderTransform::DecodeOneFrame()
{
	auto& packet = _inputPacket;
	auto ret = [&] {
		if (packet.size == 0)
			return false;

		int got_frame = 0;
		while (!got_frame)
		{
			auto ret = avcodec_decode_video2(_codecContext.get(), _frame.get(), &got_frame, &packet);
			if (ret < 0 || (ret == 0 && !got_frame))
				return false;
			packet.size -= ret;
			packet.data += ret;
		}
		return true;
	}();
	if (!ret && _inputBuffer)
	{
		_inputBuffer->Unlock();
		_inputBuffer.Reset();
	}
	return ret;
}

WRL::ComPtr<IMFMediaType> FFmpegVideoDecoderTransform::OnGetOutputAvailableType(DWORD index) noexcept
{
	if (index < availableOutputTypes.size())
	{
		return availableOutputTypes[index];
	}
	else
		return nullptr;
}

void FFmpegVideoDecoderTransform::InitializeAvailableOutputTypes(IMFMediaType* inputType)
{
	availableOutputTypes.clear();

	ComPtr<IMFMediaType> outputMediaType;
	ThrowIfFailed(MFCreateMediaType(&outputMediaType));
	ThrowIfFailed(inputType->CopyAllItems(outputMediaType.Get()));
	ThrowIfFailed(outputMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YV12));
	availableOutputTypes.emplace_back(std::move(outputMediaType));
}

void FFmpegVideoDecoderTransform::InitializeDecoder(IMFMediaType* inputType)
{
	ThrowIfFailed(MFGetAttributeSize(inputType, MF_MT_FRAME_SIZE, &_width, &_height));
	ThrowIfFailed(inputType->GetUINT64(MF_MT_FRAME_RATE, reinterpret_cast<UINT64*>(&_framerate)));
	ThrowIfFailed(inputType->GetUINT64(MF_MT_PIXEL_ASPECT_RATIO, reinterpret_cast<UINT64*>(&_aspectRatio)));

	ComPtr<IUnknown> optionsUnk;
	ThrowIfFailed(inputType->GetUnknown(MF_MT_LIBAV_VIDEO_CODEC_OPTIONS, IID_PPV_ARGS(&optionsUnk)));
	auto options = static_cast<LibAVVideoCodecOptions*>(optionsUnk.Get());

	auto codec = avcodec_find_decoder(options->CodecId);
	ThrowIfNot(codec, L"Cannot find a codec.");
	_codecContext.reset(avcodec_alloc_context3(codec));
	ThrowIfNot(_codecContext, L"Cannot allocate codec context.");
	_codecContext->pix_fmt = options->VideoPixelFormat;
	_codecContext->width = _width;
	_codecContext->height = _height;
	_codecContext->framerate = AVRational{ int(_framerate.Numerator), int(_framerate.Denominator) };
	_codecContext->sample_aspect_ratio = AVRational{ int(_aspectRatio.Numerator), int(_aspectRatio.Denominator) };
	_codecContext->flags = options->Flags;
	_codecContext->flags2 = options->Flags2;

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

DWORD FFmpegVideoDecoderTransform::FillFrame(IMFMediaBuffer* buffer, BYTE * data, DWORD maxLength)
{
	auto context = sws_getContext(_frame->width, _frame->height, _codecContext->pix_fmt, _width, _height, _outputFormat, 0, nullptr, nullptr, nullptr);
	if (context != _swsContext.get())
	{
		_swsContext.reset(context);
		ThrowIfNot(context && sws_init_context(context, nullptr, nullptr) >= 0, L"Cannot load resampler.");
	}
	ThrowIfNot(context, L"Cannot load resampler.");
	uint8_t* out[] = { data, data + _width * _height * 10 / 8, data + _width * _height};
	int linesize[] = { _width, _width / 2, _width / 2 };
	auto converted = sws_scale(context, _frame->data, _frame->linesize, 0, _height, out, linesize);

	//int converted = 0;
	//uint8_t* out[] = { data };
	//converted = swr_convert(swrContext, out, _frame->nb_samples, (const uint8_t**)_frame->extended_data, _frame->nb_samples);
	ThrowIfNot(converted == _height, L"Resample failed.");
	av_frame_unref(_frame.get());
	return avpicture_get_size(_outputFormat, _width, converted);
}

void FFmpegVideoDecoderTransform::BeginStreaming()
{
}