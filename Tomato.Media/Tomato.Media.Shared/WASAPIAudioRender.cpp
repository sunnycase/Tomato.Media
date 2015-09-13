//
// Tomato Media
// WASAPI 音频渲染器
// 
// 作者：SunnyCase 
// 创建日期 2015-09-04
#include "pch.h"
#include "WASAPIAudioRender.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

namespace
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	auto CreateAudioClient()
	{
		ComPtr<IMMDeviceEnumerator> mmDeviceEnumerator;
		ThrowIfFailed(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
			IID_PPV_ARGS(&mmDeviceEnumerator)));
		ComPtr<IMMDevice> mmDevice;
		ThrowIfFailed(mmDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &mmDevice));
		ComPtr<IAudioClientT> audioClient;
		ThrowIfFailed(mmDevice->Activate(__uuidof(decltype(audioClient)::InterfaceType),
			CLSCTX_ALL, nullptr, &audioClient));
		return task_from_result(audioClient);
	}
#else
	using namespace Windows::Media::Devices;

	struct ActivateAudioInterfaceCompletionHandler : public RuntimeClass<RuntimeClassFlags<
		RuntimeClassType::ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler>
	{
	public:
		ActivateAudioInterfaceCompletionHandler()
		{
		}

		task<ComPtr<IAudioClientT>> Activate()
		{
			// Get a string representing the Default Audio Device Renderer
			auto deviceId = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);
			ComPtr<IActivateAudioInterfaceAsyncOperation> asyncOp;

			// This call must be made on the main UI thread.  Async operation will call back to
			// IActivateAudioInterfaceCompletionHandler::ActivateCompleted, which must be an agile interface implementation
			ThrowIfFailed(ActivateAudioInterfaceAsync(deviceId->Data(),
				__uuidof(IAudioClientT), nullptr, this, &asyncOp));
			return create_task(completionEvent);
		}

		// 通过 RuntimeClass 继承
		STDMETHODIMP ActivateCompleted(IActivateAudioInterfaceAsyncOperation * activateOperation) noexcept override
		{
			try
			{
				HRESULT hr;
				ComPtr<IUnknown> audioClientUnk;
				ThrowIfFailed(activateOperation->GetActivateResult(&hr, &audioClientUnk));
				ThrowIfFailed(hr);

				ComPtr<IAudioClient2> audioClient;
				ThrowIfFailed(audioClientUnk.As(&audioClient));
				completionEvent.set(audioClient);
			}
			CATCH_ALL_WITHEVENT(completionEvent);
			return S_OK;
		}
	private:
		task_completion_event<ComPtr<IAudioClient2>> completionEvent;
	};

	auto CreateAudioClient()
	{
		auto handler = Make<ActivateAudioInterfaceCompletionHandler>();
		return handler->Activate();
	}
#endif
}

WASAPIAudioRender::WASAPIAudioRender()
	:sampleRequestEvent(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS)),
	workerQueue(MFASYNC_CALLBACK_QUEUE_UNDEFINED),
	operationQueue(std::make_shared<decltype(operationQueue)::element_type>(
		[weak = AsWeak()](TOperation& op)
{
	if (auto me = weak.Resolve()) me->OnDispatchOperation(op);
}))
{
	ThrowWin32IfNot(sampleRequestEvent.IsValid());
}

#ifdef AudioClient_Support_ClientProperties

AUDIO_STREAM_CATEGORY WASAPIAudioRender::GetAudioCategory() const noexcept
{
	return audioClientProps.eCategory;
}

void WASAPIAudioRender::SetAudioCategory(AUDIO_STREAM_CATEGORY value)
{
	audioClientProps.eCategory = value;
	ThrowIfFailed(audioClient->SetClientProperties(&audioClientProps));
}

#endif

task<void> WASAPIAudioRender::Initialize()
{
	if (!initStarted.exchange(true))
		initTask = InitializeCore();
	return initTask;
}

void WASAPIAudioRender::SetWorkerQueueProvider(NS_CORE::MFWorkerQueueProviderRef workerQueue)
{
	this->workerQueue = workerQueue;
	RegisterWorkThreadIfNeeded();
}

void WASAPIAudioRender::SetIsActive(bool value)
{
	if (value)
	{
		if (initTask.is_done())
			ThrowIfFailed(E_NOT_VALID_STATE);
		initTask.get();
		if (!isActive.load(std::memory_order_consume))
		{
			isActive.store(true, std::memory_order_release);
			operationQueue->Enqueue(std::make_shared<Operation>(OperationKind::Start));
		}
	}
	else
	{
		if (isActive.load(std::memory_order_consume))
		{
			isActive.store(false, std::memory_order_release);
			operationQueue->Enqueue(std::make_shared<Operation>(OperationKind::Stop));
		}
	}
}

void WASAPIAudioRender::SetProvideSampleCallback(std::function<size_t(byte*, size_t)>&& callback)
{
	operationQueue->Enqueue(std::make_shared<SetSampleCallbackOperation>(std::move(callback)));
}

task<void> WASAPIAudioRender::InitializeCore()
{
	return CreateDeviceResources();
}

task<void> WASAPIAudioRender::CreateDeviceResources()
{
	ComPtr<WASAPIAudioRender> thisGuard(this);
	return CreateAudioClient().then([thisGuard, this]
		(decltype(WASAPIAudioRender::audioClient) audioClient)
	{
		this->audioClient = audioClient;
		ConfigureDevice();
	}).then([thisGuard, this](task<void> t)
	{
		try
		{
			t.get();
		}
		catch (...)
		{
			initStarted.store(false, std::memory_order_release);
			throw;
		}
	});
}

void WASAPIAudioRender::ConfigureDevice()
{
	if (!audioClient) ThrowIfFailed(E_NOT_VALID_STATE);

#ifdef AudioClient_Support_ClientProperties
	ThrowIfFailed(audioClient->SetClientProperties(&audioClientProps));
#endif
	// 获取设备输入格式
	WAVEFORMATEX* mixFormat;
	ThrowIfFailed(audioClient->GetMixFormat(&mixFormat));
	this->mixFormat.reset(mixFormat);
	// 初始化 AudioClient
	ThrowIfFailed(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
		0, 0, mixFormat, nullptr));
	// 获取设备缓冲大小
	ThrowIfFailed(audioClient->GetBufferSize(&deviceBufferFrames));
	// 获取音频渲染客户端
	ThrowIfFailed(audioClient->GetService(IID_PPV_ARGS(&audioRenderClient)));
	// 设置采样请求 Event
	ThrowIfFailed(audioClient->SetEventHandle(sampleRequestEvent.Get()));
}

void WASAPIAudioRender::OnDispatchOperation(TOperation & op)
{
	switch (op->Kind)
	{
	case OperationKind::Start:
		DoStart();
		break;
	case OperationKind::Stop:
		DoStop();
		break;
	case OperationKind::SampleRequest:
		OnSampleRequested();
		break;
	case OperationKind::SetSampleCallback:
		OnSetProvideSampleCallback(std::move(static_cast<SetSampleCallbackOperation*>(op.get())->Callback));
		break;
	default:
		ThrowIfFailed(E_UNEXPECTED);
	}
}

void WASAPIAudioRender::RegisterWorkThreadIfNeeded()
{
	if (workThreadRegistered) return;

	if (!workerQueue.IsValid())
		workerQueue = Core::MFWorkerQueueProvider::GetAudio();

	operationQueue->SetWorkerQueue(workerQueue);
	workThreadRegistered = true;
}

UINT32 WASAPIAudioRender::GetBufferFramesAvailable()
{
	UINT32 paddingFrames = 0;
	// Get padding in existing buffer
	ThrowIfFailed(audioClient->GetCurrentPadding(&paddingFrames));

	// In non-HW shared mode, GetCurrentPadding represents the number of queued frames
	// so we can subtract that from the overall number of frames we have
	// 否则返回的是缓冲中已被使用的帧数
	return deviceBufferFrames - paddingFrames;
}

void WASAPIAudioRender::FillBufferAvailable(bool isSilent)
{
	auto framesAvailable = GetBufferFramesAvailable();
	// 没有可用空间则直接返回
	if (!framesAvailable) return;

	if (isSilent || !sampleCallback)
	{
		// 用空白填充缓冲
		byte* buffer = nullptr;
		ThrowIfFailed(audioRenderClient->GetBuffer(framesAvailable, &buffer));
		ThrowIfFailed(audioRenderClient->ReleaseBuffer(framesAvailable, AUDCLNT_BUFFERFLAGS_SILENT));
	}
	// Even if we cancel a work item, this may still fire due to the async
	// nature of things.  There should be a queued work item already to handle
	// the process of stopping or stopped
	else if (isActive.load(std::memory_order_consume))
		FillBufferFromMediaSource(framesAvailable);
}

void WASAPIAudioRender::FillBufferFromMediaSource(UINT32 framesCount)
{
	BYTE* data = nullptr;
	UINT32 actualBytesRead = 0;
	UINT32 actualBytesToRead = framesCount * mixFormat->nBlockAlign;

	//ThrowIfFailed(audioRenderClient->GetBuffer(framesCount, &data));
	//if (actualBytesRead = sourceReader->Read(data, actualBytesToRead))
	//{
	//	auto framesRead = actualBytesRead / mixFormat->nBlockAlign;
	//	ThrowIfFailed(audioRenderClient->ReleaseBuffer(framesRead, 0));
	//}
	//// 未读出数据
	//else
	//{
	//	ThrowIfFailed(audioRenderClient->ReleaseBuffer(framesCount, AUDCLNT_BUFFERFLAGS_SILENT));
	//	// 媒体结束
	//	if (sourceReader->GetState() == SourceReaderState::Ended)
	//		OnMediaEnded();
	//}
}

size_t WASAPIAudioRender::GetBufferFramesPerPeriod()
{
	using namespace std::chrono;
	REFERENCE_TIME defaultDevicePeriod = 0;
	REFERENCE_TIME minimumDevicePeriod = 0;

	// Get the audio device period
	ThrowIfFailed(audioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod));

	// 100 ns = 1e-7 s
	auto devicePeriodSecs = duration_cast<duration<double>>(hnseconds(defaultDevicePeriod));
	return static_cast<UINT32>(mixFormat->nSamplesPerSec * devicePeriodSecs.count() + 0.5);
}

void WASAPIAudioRender::DoStart()
{
	InitializeDeviceBuffer();
	auto hr = audioClient->Start();
	if (hr != AUDCLNT_E_NOT_STOPPED && FAILED(hr))
	{
		isActive.store(false, std::memory_order_release);
		ThrowIfFailed(hr);
	}
	operationQueue->Enqueue(std::make_shared<Operation>(OperationKind::SampleRequest), sampleRequestEvent);
}

void WASAPIAudioRender::DoStop()
{
	if (audioClient = this->audioClient)
	{
		auto hr = audioClient->Stop();
		if (FAILED(hr))
		{
			isActive.store(true, std::memory_order_release);
			ThrowIfFailed(hr);
		}
	}
}

void WASAPIAudioRender::OnSetProvideSampleCallback(std::function<size_t(byte*, size_t)>&& callback)
{
	this->sampleCallback = std::move(callback);
}

void WASAPIAudioRender::InitializeDeviceBuffer()
{
	FillBufferAvailable(true);
}

void WASAPIAudioRender::OnSampleRequested()
{
	try
	{
		FillBufferAvailable();
	}
	catch (...)
	{
	}
	if (isActive.load(std::memory_order_release))
		operationQueue->Enqueue(std::make_shared<Operation>(OperationKind::SampleRequest), sampleRequestEvent);
}
