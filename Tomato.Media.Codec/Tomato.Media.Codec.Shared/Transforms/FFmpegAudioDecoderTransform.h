//
// Tomato Media Codec
// Media Foundation FFmpeg Audio 解码 Transform
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#pragma once
#include "DecoderTransformBase.h"
#if (NTDDI_VERSION >= NTDDI_WIN8)
#include "Tomato.Media.Codec_i.h"
#endif
#include <vector>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif
#include "Utilities/FFmpegHelper.h"

DEFINE_NS_MEDIA_CODEC

// Media Foundation FFmpeg Audio 解码 Transform
class FFmpegAudioDecoderTransform : public DecoderTransformBase
{
	enum class VorbisState
	{
		Header,
		Body
	};
#if (NTDDI_VERSION >= NTDDI_WIN8)
	InspectableClass(RuntimeClass_Tomato_Media_Codec_FFmpegAudioDecoderTransform, BaseTrust)
#endif
public:
	static const DecoderTransformRegisterInfo RegisterInfos[2];

	FFmpegAudioDecoderTransform();

private:
	virtual void OnValidateInputType(IMFMediaType* type) override;
	virtual void OnValidateOutputType(IMFMediaType* type) override;
	// 获取输出帧大小
	virtual DWORD OnGetOutputFrameSize() const noexcept override;
	// 设置输入类型
	virtual WRL::ComPtr<IMFMediaType> OnSetInputType(IMFMediaType* type) override;
	// 设置输出类型
	virtual WRL::ComPtr<IMFMediaType> OnSetOutputType(IMFMediaType* type) override;
	// 接收输入
	virtual bool OnReceiveInput(IMFSample* sample) override;
	virtual void OnProduceOutput(MFT_OUTPUT_DATA_BUFFER& output) override;
	virtual WRL::ComPtr<IMFMediaType> OnGetOutputAvailableType(DWORD index) noexcept override;
	virtual void BeginStreaming() override;

	void InitializeAvailableOutputTypes();
	void InitializeDecoder(IMFMediaType* inputType);
	//bool FeedBodyPacket(ogg_packet& packet);
	DWORD FillFloatFrame(IMFMediaBuffer* buffer, BYTE* data, DWORD maxLength);
private:
	std::vector<WRL::ComPtr<IMFMediaType>> availableOutputTypes;
	FFmpeg::unique_avcodeccontext _codecContext;
	unique_cotaskmem<FFmpeg::WAVEFORMATLIBAV> _waveFormat;

	size_t bytesPerDecodecSample = 0;
	size_t decodedSamples = 0;
};

END_NS_MEDIA_CODEC