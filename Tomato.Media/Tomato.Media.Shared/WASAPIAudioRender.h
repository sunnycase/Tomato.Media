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
#include "../../include/WeakReferenceBase.h"
#include "../../include/MFOperationQueue.h"

DEFINE_NS_MEDIA

#if (WINVER >= _WIN32_WINNT_WIN8)
typedef IAudioClient2 IAudioClientT;
#define AudioClient_Support_ClientProperties
#else
typedef IAudioClient IAudioClientT;
#endif

///<summary>音频渲染器</summary>
class WASAPIAudioRender : public Core::WeakReferenceBase<WASAPIAudioRender,
	WRL::RuntimeClassFlags<WRL::ClassicCom>, IAudioRender>
{
	enum class OperationKind
	{
		Start,
		Stop,
		SampleRequest,
		SetSampleCallback
	};

	struct Operation
	{
		OperationKind Kind;

		Operation(OperationKind kind)
			:Kind(kind)
		{}
	};

	struct SetSampleCallbackOperation : public Operation
	{
		std::function<size_t(byte*, size_t)> Callback;

		template<typename T>
		SetSampleCallbackOperation(T&& callback)
			:Operation(OperationKind::SetSampleCallback),
			Callback(std::forward<T>(callback))
		{}
	};

	typedef std::shared_ptr<Operation> TOperation;
public:
	WASAPIAudioRender();

#ifdef AudioClient_Support_ClientProperties
	AUDIO_STREAM_CATEGORY GetAudioCategory() const noexcept;
	void SetAudioCategory(AUDIO_STREAM_CATEGORY value);
#endif
	virtual concurrency::task<void> Initialize() override;
	virtual void SetWorkerQueueProvider(Core::MFWorkerQueueProviderRef workerQueue) override;
	virtual void SetIsActive(bool value) override;
	virtual void SetProvideSampleCallback(std::function<size_t(byte*, size_t)>&& callback) override;
private:
	concurrency::task<void> InitializeCore();
	concurrency::task<void> CreateDeviceResources();
	void ConfigureDevice();
	void OnDispatchOperation(TOperation& op);

	///<remarks>调用前需对状态加锁</remarks>
	void RegisterWorkThreadIfNeeded();
	void OnSampleRequested();
	void FillBufferAvailable(bool isSilent = false);
	void InitializeDeviceBuffer();
	// 获取设备剩余缓冲帧数
	UINT32 GetBufferFramesAvailable();
	void FillBufferFromMediaSource(UINT32 framesCount);
	size_t GetBufferFramesPerPeriod();

	void DoStart();
	void DoStop();
	void OnSetProvideSampleCallback(std::function<size_t(byte*, size_t)>&& callback);
private:
	std::atomic_bool initStarted = false;
	concurrency::task<void> initTask;
	unique_cotaskmem<WAVEFORMATEX> mixFormat;
	WRL::Wrappers::Event sampleRequestEvent;
	UINT32 deviceBufferFrames;
	bool workThreadRegistered = false;
	std::atomic<bool> isActive = false;
	Core::MFWorkerQueueProviderRef workerQueue;
	std::shared_ptr<Core::MFOperationQueue<TOperation, true>> operationQueue;
	std::function<size_t(byte*, size_t)> sampleCallback;

	WRL::ComPtr<IAudioClientT> audioClient;
	WRL::ComPtr<IAudioRenderClient> audioRenderClient;
#ifdef AudioClient_Support_ClientProperties
	AudioClientProperties audioClientProps =
	{
		sizeof(audioClientProps),
		FALSE,
#if (NTDDI_VERSION >= NTDDI_WINTHRESHOLD)
		AudioCategory_Media,
#else
		AudioCategory_BackgroundCapableMedia
#endif
	};
#endif
};

END_NS_MEDIA