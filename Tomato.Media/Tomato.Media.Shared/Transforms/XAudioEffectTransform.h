//
// Tomato Media
// XAudio 音效 for MFTransform
// 
// 作者：SunnyCase 
// 创建日期 2015-01-15
#pragma once
#include "common.h"
#if WINAPI_FAMILY==WINAPI_FAMILY_APP
#include "Tomato.Media_i.h"
#endif
#include "../../include/media/Transforms/EffectTransformBase.h"
#include <XAudio2.h>
#include <XAudio2fx.h>
#include "../../include/XAudio2RAII.h"

DEFINE_NS_MEDIA
#if WINAPI_FAMILY!=WINAPI_FAMILY_APP
#include "Tomato.Media_i.h"
#endif

class XAudioEffectTransform : public EffectTransformBase
{
#if (WINAPI_FAMILY==WINAPI_FAMILY_APP)
	InspectableClass(RuntimeClass_Tomato_Media_XAudioEffectTransform, BaseTrust)
#endif
public:
	XAudioEffectTransform();
protected:
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

	void InitializeXAudio();
	void InitializeAvailableOutputTypes();
	void InitializeSourceVoice(IMFMediaType* inputType);
	void InitializeEffectChain(IMFMediaType* outputType);
	//bool FeedBodyPacket(ogg_packet& packet);
	DWORD FillFrame(IMFMediaBuffer* buffer, BYTE* data, DWORD maxLength);
	bool DecodeOneFrame();
private:
	WRL::ComPtr<IXAudio2> _xAudio;
	std::vector<WRL::ComPtr<IMFMediaType>> availableOutputTypes;
	WRL::ComPtr<IMFMediaBuffer> _inputBuffer;
	std::unique_ptr<IXAudio2SourceVoice, Core::xaudio2_voice_deleter> _sourceVoice;
	std::unique_ptr<IXAudio2SubmixVoice, Core::xaudio2_voice_deleter> _submixVoice;

	UINT32 _outputChannels;
	UINT32 _outputSampleRate;
	UINT32 _outputBlockAlign;
	size_t bytesPerDecodecSample = 0;
	size_t decodedSamples = 0;
	MFTIME _sampleTime = -1;
};

END_NS_MEDIA