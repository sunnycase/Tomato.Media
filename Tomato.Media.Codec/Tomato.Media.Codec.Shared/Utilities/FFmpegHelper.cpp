//
// Tomato Media Codec
// FFmpeg 辅助
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#include "pch.h"
#include "FFmpegHelper.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/log.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

using namespace NS_MEDIA;
using namespace NS_MEDIA_CODEC;
using namespace NS_MEDIA_CODEC::FFmpeg;
using namespace NS_MEDIA_CODEC::FFmpeg::Wrappers;
using namespace WRL;

namespace
{
	struct FFmpegInitializer
	{
		FFmpegInitializer()
		{
			av_log_set_callback(AVLogCallback);
			av_register_all();
			avcodec_register_all();
		}

		static void AVLogCallback(void* avcl, int level, const char* fmt, va_list vargs)
		{
#if _DEBUG
			char buffer[INT16_MAX];
			vsprintf_s(buffer, fmt, vargs);
			OutputDebugStringA(buffer);
#endif
		}
	};
}

void FFmpeg::Initialize()
{
	static FFmpegInitializer init;
}

MFAVIOContext::MFAVIOContext(IMFByteStream * byteStream, size_t bufferSize, bool canWrite)
	:_byteStream(byteStream), _ioContext(avio_alloc_context(reinterpret_cast<unsigned char*>(av_malloc(bufferSize)),
		static_cast<int>(bufferSize), canWrite ? 1 : 0, this, ReadPacket, canWrite ? WritePacket : nullptr, Seek))
{
	ThrowIfNot(_ioContext, L"Create MF AVIOContext Failed.");
}

AVFormatContextWrapper MFAVIOContext::OpenFormatContext()
{
	AVFormatContextWrapper fmtContext(avformat_alloc_context());
	fmtContext.Open(shared_from_this(), _ioContext.Get());
	return std::move(fmtContext);
}

int MFAVIOContext::ReadPacket(void * opaque, uint8_t * buf, int buf_size) noexcept
{
	assert(buf_size >= 0);
	auto ctx = reinterpret_cast<MFAVIOContext*>(opaque);
	assert(ctx);
	if (auto stream = ctx->_byteStream)
	{
		try
		{
			QWORD len = 0, pos = 0;
			ThrowIfFailed(ctx->_byteStream->GetLength(&len));
			ThrowIfFailed(ctx->_byteStream->GetCurrentPosition(&pos));
			auto toRead = static_cast<ULONG>(std::min(len - pos, static_cast<QWORD>(buf_size)));

			ULONG read = 0;
			ThrowIfFailed(ctx->_byteStream->Read((BYTE*)buf, toRead, &read));
			return static_cast<int>(read);
		}
		catch (...)
		{
		}
	}
	return -1;
}

int MFAVIOContext::WritePacket(void * opaque, uint8_t * buf, int buf_size) noexcept
{
	assert(false);
	return 0;
}

int64_t MFAVIOContext::Seek(void * opaque, int64_t offset, int whence) noexcept
{
	auto ctx = reinterpret_cast<MFAVIOContext*>(opaque);
	assert(ctx);
	if (auto stream = ctx->_byteStream)
	{
		try
		{
			QWORD currentPos = 0;
			// 获取流长度
			if (whence == AVSEEK_SIZE)
			{
				QWORD len = 0;
				ThrowIfFailed(stream->GetLength(&len));
				return static_cast<int64_t>(len);
			}
			// 从头部开始
			else if (whence == SEEK_SET)
			{
				ThrowIfFailed(stream->Seek(msoBegin, static_cast<LONGLONG>(offset),
					MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &currentPos));
			}
			else if (whence == SEEK_CUR)
			{
				ThrowIfFailed(stream->Seek(msoCurrent, static_cast<LONGLONG>(offset),
					MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &currentPos));
			}
			else if (whence == SEEK_END)
			{
				QWORD len = 0;
				ThrowIfFailed(stream->GetLength(&len));
				ThrowIfFailed(stream->Seek(msoBegin, len + offset, MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &currentPos));
			}
			else
			{
				ThrowIfFailed(E_INVALIDARG);
			}
			return currentPos;
		}
		catch (...)
		{
		}
	}
	return -1;
}

AVIOContextWrapper::AVIOContextWrapper() noexcept
	:_context(nullptr)
{

}

AVIOContextWrapper::AVIOContextWrapper(AVIOContext * context) noexcept
	: _context(context)
{
}

AVIOContextWrapper::~AVIOContextWrapper()
{
	av_freep(&_context);
}

AVFormatContextWrapper::AVFormatContextWrapper(AVFormatContext * context) noexcept
	: _context(context)
{
}

AVFormatContextWrapper::AVFormatContextWrapper(AVFormatContextWrapper && other) noexcept
	: _context(std::exchange(other._context, nullptr)), _opened(std::exchange(other._opened, false)),
	_opaque(std::exchange(other._opaque, nullptr))
{

}

AVFormatContextWrapper::~AVFormatContextWrapper()
{
	if (_opened)
		avformat_close_input(&_context);
	else
	{
		avformat_free_context(_context);
		_context = nullptr;
	}
}

void AVFormatContextWrapper::Open(std::shared_ptr<void>&& opaque, AVIOContext* context)
{
	if (_opened)
		ThrowAlways(L"Format is already opened.");
	try
	{
		_context->pb = context;
		_opaque = opaque;
		ThrowIfNot(avformat_open_input(&_context, nullptr, nullptr, nullptr) == 0, L"Open file error.");
		_opened = true;
	}
	catch (...)
	{
		_context->pb = nullptr;
	}
}

WAVEFORMATLIBAV WAVEFORMATLIBAV::CreateFromStream(AVStream* stream)
{
	auto codecContext = stream->codec;

	WAVEFORMATLIBAV format;
	format.Format.nChannels = codecContext->channels;
	format.Format.nSamplesPerSec = codecContext->sample_rate;
	switch (codecContext->sample_fmt)
	{
	case AV_SAMPLE_FMT_U8:
	case AV_SAMPLE_FMT_U8P:
		format.Format.wBitsPerSample = 8;
		format.DesiredDecodedFormat = WAVE_FORMAT_PCM;
		break;
	case AV_SAMPLE_FMT_S16:
	case AV_SAMPLE_FMT_S16P:
		format.Format.wBitsPerSample = 16;
		format.DesiredDecodedFormat = WAVE_FORMAT_PCM;
		break;
	case AV_SAMPLE_FMT_S32:
	case AV_SAMPLE_FMT_S32P:
		format.Format.wBitsPerSample = 32;
		format.DesiredDecodedFormat = WAVE_FORMAT_PCM;
		break;
	case AV_SAMPLE_FMT_FLT:
	case AV_SAMPLE_FMT_FLTP:
		format.Format.wBitsPerSample = 32;
		format.DesiredDecodedFormat = WAVE_FORMAT_IEEE_FLOAT;
		break;
	case AV_SAMPLE_FMT_DBL:
	case AV_SAMPLE_FMT_DBLP:
		format.Format.wBitsPerSample = 64;
		format.DesiredDecodedFormat = WAVE_FORMAT_IEEE_FLOAT;
		break;
	default:
		ThrowIfFailed(E_INVALIDARG);
		break;
	}

	format.Format.nBlockAlign = (format.Format.wBitsPerSample / 8) * format.Format.nChannels;
	format.Format.nAvgBytesPerSec = format.Format.nBlockAlign * format.Format.nSamplesPerSec;
	format.dwChannelMask = 0;
	format.Samples.wReserved = 0;

	format.CodecId = codecContext->codec_id;
	format.SampleFormat = codecContext->sample_fmt;
	format.BitsPerCodedSample = codecContext->bits_per_coded_sample;
	format.Flags = codecContext->flags;
	format.Flags2 = codecContext->flags2;

	return format;
}

ComPtr<LibAVCodecOptions> LibAVCodecOptions::CreateFromStream(AVStream* stream)
{
	auto codecContext = stream->codec;
	auto options = Make<LibAVCodecOptions>();

	if (codecContext->extradata_size)
		options->ExtraData.assign(codecContext->extradata, codecContext->extradata + codecContext->extradata_size);
	return std::move(options);
}

void avcodeccontext_deleter::operator()(AVCodecContext * handle) const noexcept
{
	avcodec_free_context(&handle);
}

void avframe_deleter::operator()(AVFrame* handle) const noexcept
{
	av_frame_free(&handle);
}

void swrcontext_deleter::operator()(SwrContext* handle) const noexcept
{
	swr_free(&handle);
}