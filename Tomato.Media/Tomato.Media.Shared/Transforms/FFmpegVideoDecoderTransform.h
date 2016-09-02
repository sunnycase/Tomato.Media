//
// Tomato Media Codec
// Media Foundation FFmpeg Video 解码 Transform
// 
// 作者：SunnyCase
// 创建时间：2016-05-18
#pragma once
#include "DecoderTransformBase.h"
#include "Tomato.Media_i.h"
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

// Media Foundation FFmpeg Video 解码 Transform
class FFmpegVideoDecoderTransform : public DecoderTransformBase
{
	enum class VorbisState
	{
		Header,
		Body
	};
#if (NTDDI_VERSION >= NTDDI_WIN8)
	InspectableClass(RuntimeClass_Tomato_Media_Codec_FFmpegVideoDecoderTransform, BaseTrust)
#endif
public:
	static const DecoderTransformRegisterInfo RegisterInfos[1];

	FFmpegVideoDecoderTransform();
	virtual ~FFmpegVideoDecoderTransform();

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

	void InitializeAvailableOutputTypes(IMFMediaType* inputType);
	void InitializeDecoder(IMFMediaType* inputType);
	//bool FeedBodyPacket(ogg_packet& packet);
	DWORD FillFrame(IMFMediaBuffer* buffer, BYTE* data, DWORD maxLength);
	bool DecodeOneFrame();
private:
	std::vector<WRL::ComPtr<IMFMediaType>> availableOutputTypes;
	FFmpeg::unique_avcodeccontext _codecContext;
	FFmpeg::unique_avframe _frame;
	FFmpeg::unique_swscontext _swsContext;
	UINT32 _width;
	UINT32 _height;
	MFRatio _framerate, _aspectRatio;
	AVPixelFormat _outputFormat;

	WRL::ComPtr<IMFMediaBuffer> _inputBuffer;
	AVPacket _inputPacket;

	MFTIME _sampleTime = -1;
	bool _opended = false;
};

END_NS_MEDIA_CODEC