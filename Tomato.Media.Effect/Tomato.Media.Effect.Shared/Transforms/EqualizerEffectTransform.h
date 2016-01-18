//
// Tomato Media Effect
// 均衡器 MFTransform
// 
// 作者：SunnyCase 
// 创建日期 2015-01-15
#pragma once
#include "common.h"
#if WINAPI_FAMILY==WINAPI_FAMILY_APP
#include "Tomato.Media.Effect_i.h"
#endif
#include "../../include/media/Transforms/EffectTransformBase.h"
#include "Algorithms/EqualizerFilter.h"
#include <mutex>
#include <map>

DEFINE_NS_MEDIA_EFFECT
#if WINAPI_FAMILY!=WINAPI_FAMILY_APP
#include "Tomato.Media.Effect_i.h"
#endif

class EqualizerEffectTransform : public WRL::RuntimeClass<WRL::RuntimeClassFlags<
#if WINAPI_FAMILY==WINAPI_FAMILY_APP
	WRL::WinRtClassicComMix>,
	ABI::Tomato::Media::Effect::IEqualizerEffectProps,
#else
	WRL::ClassicCom>,
	IEqualizerEffectProps,
#endif
	EffectTransformBase>
{
	struct FilterConfig
	{
		float frequency, bandWidth, gain;
	};
#if (WINAPI_FAMILY==WINAPI_FAMILY_APP)
	InspectableClass(RuntimeClass_Tomato_Media_Effect_EqualizerEffectTransform, BaseTrust)
#endif
public:
	EqualizerEffectTransform();

	STDMETHODIMP STDMETHODCALLTYPE AddOrUpdateFilter(FLOAT frequency, FLOAT bandWidth, FLOAT gain) override;
	STDMETHODIMP STDMETHODCALLTYPE RemoveFilter(FLOAT frequency) override;
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
	virtual WRL::ComPtr<IMFMediaType> OnGetInputAvailableType(DWORD index) noexcept override;
	virtual WRL::ComPtr<IMFMediaType> OnGetOutputAvailableType(DWORD index) noexcept override;
	virtual void BeginStreaming() override;

	void InitializeAvailableOutputTypes(IMFMediaType * inputType);
	void InitializeEffectChain();
	DWORD FillFrame(BYTE* source, DWORD sourceSize, BYTE* dest, DWORD destSize);
private:
	std::vector<WRL::ComPtr<IMFMediaType>> availableOutputTypes;
	WRL::ComPtr<IMFMediaBuffer> _inputBuffer;
	std::vector<MultiBandEqualizerFilter> _channelFilters;
	std::mutex _filtersMutex;
	std::map<float, FilterConfig> _filterConfigs;
	bool _filtersConfigsDirty;

	UINT32 _outputChannels;
	UINT32 _outputSampleRate;
	UINT32 _outputBlockAlign;
	DWORD _inputSize = 0;
	MFTIME _sampleTime = -1;
	MFTIME _sampleDuration = -1;
};

END_NS_MEDIA_EFFECT