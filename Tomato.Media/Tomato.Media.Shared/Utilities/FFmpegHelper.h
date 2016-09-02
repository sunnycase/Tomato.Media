//
// Tomato Media Codec
// FFmpeg 辅助
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#pragma once
#include <Tomato.Media/Tomato.Media.h>
#include <Tomato.Core/NonCopyable.h>
#include <mfidl.h>
#include "constants.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

DEFINE_NS_MEDIA_CODEC

namespace FFmpeg
{
	inline REFERENCE_TIME dt2hns(int64_t dt, AVStream* stream)
	{
		auto sec_base = stream ? (float)stream->time_base.num / stream->time_base.den : 1.f / AV_TIME_BASE;
		auto hns = dt * sec_base * 1e7;
		return static_cast<REFERENCE_TIME>(hns);
	}

	inline int64_t hns2dt(REFERENCE_TIME hns, AVStream* stream)
	{
		auto sec_base = stream ? (float)stream->time_base.num / stream->time_base.den : 1.f / AV_TIME_BASE;
		auto dt = hns / sec_base / 1e7;
		return static_cast<int64_t>(dt);
	}

	void Initialize();
	class MFAVIOContext;

	namespace Wrappers
	{
		class AVIOContextWrapper : Core::NonCopyable
		{
		public:
			AVIOContextWrapper() noexcept;
			AVIOContextWrapper(AVIOContext* context) noexcept;
			~AVIOContextWrapper();
			operator bool() const noexcept { return _context != nullptr; }
			
			void Reset() noexcept { _context = nullptr; }
			AVIOContext* Get() const noexcept { return _context; }
		private:
			AVIOContext* _context;
		};

		class AVFormatContextWrapper : Core::NonCopyable
		{
		public:
			AVFormatContextWrapper(AVFormatContext* context) noexcept;
			AVFormatContextWrapper(AVFormatContextWrapper&& other) noexcept;
			~AVFormatContextWrapper();
			operator bool() const noexcept { return _context != nullptr; }
			void Open(std::shared_ptr<void>&& opaque, AVIOContext* context);

			void Reset() noexcept { _context = nullptr; }
			AVFormatContext* Get() const noexcept { return _context; }
			AVFormatContext** GetAddress() noexcept { return &_context; }
			AVFormatContext* operator->() const noexcept { return _context; }
		private:
			AVFormatContext* _context;
			bool _opened = false;
			std::shared_ptr<void> _opaque;
		};
	}

	class MFAVIOContext : public std::enable_shared_from_this<MFAVIOContext>
	{
	public:
		MFAVIOContext(IMFByteStream* byteStream, size_t bufferSize, bool canWrite);
		virtual ~MFAVIOContext();

		Wrappers::AVFormatContextWrapper OpenFormatContext();
		Wrappers::AVIOContextWrapper& Get() { return _ioContext; }
	private:
		static int ReadPacket(void *opaque, uint8_t *buf, int buf_size) noexcept;
		static int WritePacket(void *opaque, uint8_t *buf, int buf_size) noexcept;
		static int64_t Seek(void *opaque, int64_t offset, int whence) noexcept;
	private:
		WRL::ComPtr<IMFByteStream> _byteStream;
		Wrappers::AVIOContextWrapper _ioContext;
	};

	struct WAVEFORMATLIBAV : public WAVEFORMATEXTENSIBLE
	{
		WAVEFORMATLIBAV()
		{
			this->Format = { 0 };
			this->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			this->Format.cbSize = sizeof(WAVEFORMATLIBAV) - sizeof(WAVEFORMATEX);
			this->SubFormat = MFAudioFormat_LibAV;
		}

		enum AVCodecID CodecId;
		WORD DesiredDecodedFormat;
		enum AVSampleFormat SampleFormat;
		int BitsPerCodedSample;
		int Flags, Flags2;

		static WAVEFORMATLIBAV CreateFromStream(AVStream* stream);
	private:
	};

	struct LibAVAudioCodecOptions : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IUnknown>
	{
		std::vector<uint8_t> ExtraData;

		static WRL::ComPtr<LibAVAudioCodecOptions> CreateFromStream(AVStream* stream);
	};

	struct avcodeccontext_deleter
	{
		void operator()(AVCodecContext* handle) const noexcept;
	};
	using unique_avcodeccontext = std::unique_ptr<AVCodecContext, avcodeccontext_deleter>;


	struct avframe_deleter
	{
		void operator()(AVFrame* handle) const noexcept;
	};
	using unique_avframe = std::unique_ptr<AVFrame, avframe_deleter>;

	struct swrcontext_deleter
	{
		void operator()(SwrContext* handle) const noexcept;
	};
	using unique_swrcontext = std::unique_ptr<SwrContext, swrcontext_deleter>;

	struct AVPacketRAII : public AVPacket, Core::NonCopyable
	{
		AVPacketRAII();
		~AVPacketRAII();
		AVPacketRAII(AVPacketRAII&& other) noexcept;
		AVPacketRAII& operator=(AVPacketRAII&& other) noexcept;
	};

	void CreateMFMediaBufferOnAVPacket(AVPacketRAII&& packet, IMFMediaBuffer** mediaBuffer);
}

END_NS_MEDIA_CODEC