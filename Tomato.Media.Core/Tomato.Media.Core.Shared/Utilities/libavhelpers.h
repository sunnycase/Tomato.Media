//
// Tomato Media
// FFmpeg 辅助
// 
// (c) SunnyCase 
// 创建日期 2015-03-16
#pragma once
#include "../include/tomato.media.core.h"

NSDEF_TOMATO_MEDIA

struct av_deleter
{
	void operator()(void* handle) const noexcept;
};

struct avio_deleter
{
	void operator()(AVIOContext* handle) const noexcept;
};

struct avframe_deleter
{
	void operator()(AVFrame* handle) const noexcept;
};

template<class T>
using unique_av = std::unique_ptr<T, av_deleter>;
using unique_avio = std::unique_ptr<AVIOContext, avio_deleter>;
using unique_avframe = std::unique_ptr<AVFrame, avframe_deleter>;

template<bool close>
struct avformatctx_deleter
{
	void operator()(AVFormatContext* ctx) const noexcept
	{
		if (ctx)
		{
			if (close)
			{
				auto old = ctx;
				avformat_close_input(&old);
			}
			else
				avformat_free_context(ctx);
		}
	}
};

template<bool close>
using unique_avformat = std::unique_ptr<AVFormatContext, avformatctx_deleter<close>>;

class MFAVIOContext
{
public:
	MFAVIOContext(wrl::ComPtr<IMFByteStream> stream, size_t bufferSize, bool canWrite);

	AVIOContext* Get() const noexcept { return ioctx.get(); }
private:
	static int ReadPacket(void *opaque, uint8_t *buf, int buf_size) noexcept;
	static int WritePacket(void *opaque, uint8_t *buf, int buf_size) noexcept;
	static int64_t Seek(void *opaque, int64_t offset, int whence) noexcept;
private:
	wrl::ComPtr<IMFByteStream> stream;
	unique_avio ioctx;
	unique_av<byte> iobuffer;
	bool canWrite;
};

// LibAV 可解码
extern const GUID KSDATAFORMAT_SUBTYPE_LIBAV;
// LibAV Packet
// 类型：Packet
extern const GUID MFSampleExtension_LibAVPacket;

struct WAVEFORMATLIBAV : public WAVEFORMATEXTENSIBLE
{
	AVCodecContext* CodecContext;

	WAVEFORMATLIBAV()
	{
		this->Format = { 0 };
		this->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		this->Format.cbSize = sizeof(WAVEFORMATLIBAV) - sizeof(WAVEFORMATEX);
		this->SubFormat = KSDATAFORMAT_SUBTYPE_LIBAV;
	}

	static WAVEFORMATLIBAV CreateFromFormatContext(std::shared_ptr<AVFormatContext> ctx, int streamId);
private:
	std::shared_ptr<AVFormatContext> FormatContext;
};

class MediaMetadataHelper
{
public:
	static void FillMediaMetadatas(AVIOContext* ioctx, MediaMetadataContainer& container);
	static int64_t GetDuration(AVIOContext* ioctx) noexcept;
};

void RegisterLibAV();
unique_avformat<true> OpenAVFormatContext(AVIOContext * ioctx);

NSED_TOMATO_MEDIA