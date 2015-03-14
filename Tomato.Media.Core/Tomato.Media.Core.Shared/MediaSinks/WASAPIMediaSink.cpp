//
// Tomato Media
// Media Sink
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#include "pch.h"
#include "WASAPIMediaSink.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

#if WINAPI_PARTITION_APP
#pragma comment(lib, "Mmdevapi.lib")

using namespace Platform;
using namespace Windows::Media::Devices;

class ActivateAudioInterfaceCompletionHandler : public RuntimeClass<RuntimeClassFlags<
	RuntimeClassType::ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler>
{
public:
	ActivateAudioInterfaceCompletionHandler()
	{
	}

	task<ComPtr<IAudioClient2>> Activate()
	{
		// Get a string representing the Default Audio Device Renderer
		auto deviceId = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);
		ComPtr<IActivateAudioInterfaceAsyncOperation> asyncOp;

		// This call must be made on the main UI thread.  Async operation will call back to
		// IActivateAudioInterfaceCompletionHandler::ActivateCompleted, which must be an agile interface implementation
		THROW_IF_FAILED(ActivateAudioInterfaceAsync(deviceId->Data(),
			__uuidof(IAudioClient2), nullptr, this, &asyncOp));
		return create_task(completionEvent);
	}

	// 通过 RuntimeClass 继承
	STDMETHODIMP ActivateCompleted(IActivateAudioInterfaceAsyncOperation * activateOperation) noexcept override
	{
		HRESULT hr = S_OK;

		try
		{
			ComPtr<IUnknown> audioClientUnk;
			THROW_IF_FAILED(activateOperation->GetActivateResult(&hr, &audioClientUnk));
			THROW_IF_FAILED(hr);

			ComPtr<IAudioClient2> audioClient;
			THROW_IF_FAILED(audioClientUnk.As(&audioClient));
			completionEvent.set(audioClient);
		}
		CATCH_ALL();
		return S_OK;
	}
private:
	std::function<void(ComPtr<IAudioClient2>&&)> completedHandler;
	task_completion_event<ComPtr<IAudioClient2>> completionEvent;
};
#endif

WASAPIMediaSink::WASAPIMediaSink()
	:sampleRequestEvent(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS))
{
	startPlaybackThread = mcssProvider.CreateMMCSSThread(
		std::bind(&WASAPIMediaSink::OnStartPlayback, this));
	sampleRequestedThread = mcssProvider.CreateMMCSSThread(
		std::bind(&WASAPIMediaSink::OnSampleRequested, this));
}

task<void> WASAPIMediaSink::Initialize()
{
	if (sinkState == MediaSinkState::NotInitialized)
	{
		sinkState = MediaSinkState::Initializing;
#if WINAPI_PARTITION_APP
		auto activateHandler = Make<ActivateAudioInterfaceCompletionHandler>();
		return activateHandler->Activate()
			.then([this, activateHandler](ComPtr<IAudioClient2> audioClient)
		{
			this->audioClient = audioClient;
			ConfigureDevice();

			sinkState = MediaSinkState::Ready;
		}, task_continuation_context::use_arbitrary());
#else
#error "Not support."
#endif
	}
}

void WASAPIMediaSink::StartPlayback()
{
	if (sinkState == MediaSinkState::Ready ||
		sinkState == MediaSinkState::Stopped)
		startPlaybackThread->Execute();
}

void WASAPIMediaSink::ConfigureDevice()
{
	AudioClientProperties audioProps = { 0 };
	audioProps.cbSize = sizeof(audioProps);
	audioProps.bIsOffload = FALSE;
	audioProps.eCategory = AudioCategory_BackgroundCapableMedia;

	THROW_IF_FAILED(audioClient->SetClientProperties(&audioProps));

	// 获取设备输入格式
	WAVEFORMATEX* waveFormat;
	THROW_IF_FAILED(audioClient->GetMixFormat(&waveFormat));
	deviceInputFormat.reset(waveFormat);

	// 获取设备周期
	THROW_IF_FAILED(audioClient->GetDevicePeriod(&hnsDefaultBufferDuration, nullptr));
	// 初始化 AudioClient
	THROW_IF_FAILED(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
		hnsDefaultBufferDuration, 0, deviceInputFormat.get(), nullptr));
	// 获取设备缓冲大小
	THROW_IF_FAILED(audioClient->GetBufferSize(&deviceBufferFrames));
	// 获取音频渲染客户端
	THROW_IF_FAILED(audioClient->GetService(IID_PPV_ARGS(renderClient.ReleaseAndGetAddressOf())));
	// Sets the event handle that the system signals when an audio buffer is ready to be processed by the client
	THROW_IF_FAILED(audioClient->SetEventHandle(sampleRequestEvent.Get()));
}

UINT32 WASAPIMediaSink::GetBufferFramesAvailable()
{
	UINT32 paddingFrames = 0;
	// Get padding in existing buffer
	THROW_IF_FAILED(audioClient->GetCurrentPadding(&paddingFrames));

	// In non-HW shared mode, GetCurrentPadding represents the number of queued frames
	// so we can subtract that from the overall number of frames we have
	// 否则返回的是缓冲中已被使用的帧数
	return deviceBufferFrames - paddingFrames;
}

void WASAPIMediaSink::FillBufferAvailable(bool isSilent)
{
	auto framesAvailable = GetBufferFramesAvailable();
	// 没有可用空间则直接返回
	if (!framesAvailable) return;

	if (isSilent)
	{
		// 用空白填充缓冲
		byte* buffer = nullptr;
		THROW_IF_FAILED(renderClient->GetBuffer(framesAvailable, &buffer));
		THROW_IF_FAILED(renderClient->ReleaseBuffer(framesAvailable, AUDCLNT_BUFFERFLAGS_SILENT));
	}
	// Even if we cancel a work item, this may still fire due to the async
	// nature of things.  There should be a queued work item already to handle
	// the process of stopping or stopped
	else if (sinkState == MediaSinkState::Playing)
	{
		//FillBufferMFSource(framesAvailable);
	}
}

void WASAPIMediaSink::OnStartPlayback()
{
	THROW_IF_FAILED(audioClient->Start());
	sinkState = MediaSinkState::Playing;

	sampleRequestedThread->Execute(sampleRequestEvent);
}

void WASAPIMediaSink::OnSampleRequested()
{
	// 锁定保证同时只有一个请求被处理
	std::lock_guard<decltype(sampleRequestMutex)> locker(sampleRequestMutex);
	FillBufferAvailable(true);

	if (sinkState == MediaSinkState::Playing)
	{
		// 安排下一次采样工作
		sampleRequestedThread->Execute(sampleRequestEvent);
	}
}

std::unique_ptr<IMediaSink> IMediaSink::CreateWASAPIMediaSink()
{
	return std::make_unique<WASAPIMediaSink>();
}