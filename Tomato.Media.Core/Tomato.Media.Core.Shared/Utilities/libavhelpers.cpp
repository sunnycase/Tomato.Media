//
// Tomato Media
// FFmpeg 辅助
// 
// (c) SunnyCase 
// 创建日期 2015-03-16
#include "pch.h"
#include "libavhelpers.h"
#include "encoding.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

// {0E49A298-BD9D-47E8-AC23-17AA1DD92C27}
DEFINE_GUID(NS_TOMATO_MEDIA::KSDATAFORMAT_SUBTYPE_LIBAV,
	0xe49a298, 0xbd9d, 0x47e8, 0xac, 0x23, 0x17, 0xaa, 0x1d, 0xd9, 0x2c, 0x27);

struct LibAVRegistry
{
	LibAVRegistry()
	{
		av_log_set_callback(AVLogCallback);
		av_register_all();
		avcodec_register_all();
	}

	static void AVLogCallback(void* avcl, int level, const char* fmt, va_list vargs)
	{
		char buffer[INT16_MAX];
		vsprintf_s(buffer, fmt, vargs);
		OutputDebugStringA(buffer);
	}
};

void NS_TOMATO_MEDIA::RegisterLibAV()
{
	static LibAVRegistry reg;
}

void av_deleter::operator()(void* handle) const noexcept
{
	av_free(handle);
}

MFAVIOContext::MFAVIOContext(wrl::ComPtr<IMFByteStream> stream, size_t bufferSize, bool canWrite)
	:stream(std::move(stream)), iobuffer((byte*)av_malloc(bufferSize)), canWrite(canWrite), ioctx(nullptr)
{
	ioctx = avio_alloc_context(iobuffer, (int)bufferSize, canWrite ? 1 : 0,
		this, ReadPacket, canWrite ? WritePacket : nullptr, Seek);
	THROW_IF_NOT(ioctx, "Create MF AVIOContext Failed.");
	//ioctx.reset(ctx);
}

void MFAVIOContext::Release()
{
	av_freep(&ioctx);
}

int MFAVIOContext::ReadPacket(void * opaque, uint8_t * buf, int buf_size) noexcept
{
	auto ctx = reinterpret_cast<MFAVIOContext*>(opaque);
	assert(ctx);

	try
	{
		QWORD len = 0, pos = 0;
		THROW_IF_FAILED(ctx->stream->GetLength(&len));
		THROW_IF_FAILED(ctx->stream->GetCurrentPosition(&pos));
		auto toRead = std::min(len - pos, (QWORD)buf_size);

		ULONG read = 0;
		THROW_IF_FAILED(ctx->stream->Read((BYTE*)buf, toRead, &read));
		return read;
	}
	catch (...)
	{
		return -1;
	}
}

int MFAVIOContext::WritePacket(void * opaque, uint8_t * buf, int buf_size) noexcept
{
	return 0;
}

int64_t MFAVIOContext::Seek(void * opaque, int64_t offset, int whence) noexcept
{
	auto ctx = reinterpret_cast<MFAVIOContext*>(opaque);
	auto stream = ctx->stream.Get();

	try
	{
		QWORD currentPos = 0;
		// 获取流长度
		if (whence == AVSEEK_SIZE)
		{
			QWORD len = 0;
			THROW_IF_FAILED(stream->GetLength(&len));
			return static_cast<int64_t>(len);
		}
		// 从头部开始
		else if (whence == SEEK_SET)
		{
			THROW_IF_FAILED(stream->Seek(msoBegin, static_cast<LONGLONG>(offset),
				MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &currentPos));
		}
		else if (whence == SEEK_CUR)
		{
			THROW_IF_FAILED(stream->Seek(msoCurrent, static_cast<LONGLONG>(offset),
				MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &currentPos));
		}
		else if (whence == SEEK_END)
		{
			QWORD len = 0;
			THROW_IF_FAILED(stream->GetLength(&len));
			THROW_IF_FAILED(stream->Seek(msoBegin, len + offset,
				MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &currentPos));
		}
		else
		{
			THROW_IF_FAILED(E_INVALIDARG);
		}
		return currentPos;
	}
	catch (...)
	{
		return -1;
	}
}

WAVEFORMATLIBAV WAVEFORMATLIBAV::CreateFromFormatContext(std::shared_ptr<AVFormatContext> ctx, int streamId)
{
	auto codecContext = ctx->streams[streamId]->codec;

	WAVEFORMATLIBAV format;
	format.Format.nChannels = codecContext->channels;
	format.Format.nSamplesPerSec = codecContext->sample_rate;
	switch (codecContext->sample_fmt)
	{
	case AV_SAMPLE_FMT_U8:
	case AV_SAMPLE_FMT_U8P:
		format.Format.wBitsPerSample = 8;
		break;
	case AV_SAMPLE_FMT_S16:
	case AV_SAMPLE_FMT_S16P:
		format.Format.wBitsPerSample = 16;
		break;
	case AV_SAMPLE_FMT_S32:
	case AV_SAMPLE_FMT_S32P:
	case AV_SAMPLE_FMT_FLT:
	case AV_SAMPLE_FMT_FLTP:
		format.Format.wBitsPerSample = 32;
		break;
	case AV_SAMPLE_FMT_DBL:
	case AV_SAMPLE_FMT_DBLP:
		format.Format.wBitsPerSample = 64;
		break;
	default:
		THROW_IF_FAILED(E_INVALIDARG);
		break;
	}

	format.Format.nBlockAlign = (format.Format.wBitsPerSample / 8) * format.Format.nChannels;
	format.Format.nAvgBytesPerSec = format.Format.nBlockAlign * format.Format.nSamplesPerSec;
	format.dwChannelMask = 0;
	format.Samples.wReserved = 0;

	format.FormatContext = ctx;
	format.CodecContext = codecContext;

	return format;
}

void MediaMetadataHelper::FillMediaMetadatas(AVIOContext * ioctx, MediaMetadataContainer & container)
{
	RegisterLibAV();

	auto fmtctx = avformat_alloc_context();
	fmtctx->pb = ioctx;

	THROW_IF_NOT(avformat_open_input(&fmtctx, nullptr, nullptr, nullptr) == 0, L"Open file error.");
	auto avfmtctx = unique_avformat<true>(fmtctx);
#if _DEBUG
	av_dump_format(fmtctx, 0, nullptr, 0);
#endif
	// Title
	AVDictionaryEntry* entry = nullptr;
	if (entry = av_dict_get(fmtctx->metadata, "title", nullptr, 0))
		container.Add(MediaMetadata(DefaultMediaMetadatas::Title, s2ws(entry->value)));
	if (entry = av_dict_get(fmtctx->metadata, "title", entry, 0))
		container.Add(MediaMetadata(DefaultMediaMetadatas::Title, s2ws(entry->value)));
	// Album
	if (entry = av_dict_get(fmtctx->metadata, "album", nullptr, 0))
		container.Add(MediaMetadata(DefaultMediaMetadatas::Album, s2ws(entry->value)));
	// AlbumArtist
	if (entry = av_dict_get(fmtctx->metadata, "album_artist", nullptr, 0))
		container.Add(MediaMetadata(DefaultMediaMetadatas::AlbumArtist, s2ws(entry->value)));
	// Artist
	if (entry = av_dict_get(fmtctx->metadata, "artist", nullptr, 0))
		container.Add(MediaMetadata(DefaultMediaMetadatas::Artist, s2ws(entry->value)));
}
