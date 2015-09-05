//
// Tomato Media
// WASAPI 音频渲染器
// 
// 作者：SunnyCase 
// 创建日期 2015-09-04
#pragma once
#include "IAudioRender.h"
#include <Audioclient.h>
#include <atomic>

DEFINE_NS_MEDIA

///<summary>音频渲染器</summary>
class WASAPIAudioRender : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IAudioRender>
{
public:
	WASAPIAudioRender();

	virtual concurrency::task<void> Initialize() override;
private:
	concurrency::task<void> InitializeCore();
	concurrency::task<void> CreateDeviceResources();
	void ConfigureDevice();
private:
	std::atomic_bool initStarted = false;
	concurrency::task<void> initTask;
	unique_cotaskmem<WAVEFORMATEX> mixFormat;
	WRL::Wrappers::Event sampleRequestEvent;
	UINT32 deviceBufferFrames;

#if (WINVER >= _WIN32_WINNT_WIN8)
	WRL::ComPtr<IAudioClient2> audioClient;
#elif (WINVER >= _WIN32_WINNT_VISTA)
	WRL::ComPtr<IAudioClient> audioClient;
#endif
	WRL::ComPtr<IAudioRenderClient> audioRenderClient;
};

END_NS_MEDIA