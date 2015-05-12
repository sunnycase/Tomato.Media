//
// Tomato Media
// Media Sink
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#include "pch.h"
#include "WASAPIMediaSink.h"
#include "MediaSources/RTMediaSource.h"
#include "Utilities/MFWorkerQueueProvider.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace wrl;
using namespace concurrency;

#if WINAPI_PARTITION_APP
#pragma comment(lib, "Mmdevapi.lib")

using namespace Platform;
using namespace Windows::Media::Devices;

inline double hns2s(REFERENCE_TIME time)
{
	return time * 1.0e-7;
}

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
		catch (Exception^ ex)
		{
			completionEvent.set_exception(ex);
			return ex->HResult;
		}
		catch (...)
		{
			completionEvent.set_exception(ref new COMException(E_FAIL));
			return E_FAIL;
		}
		return S_OK;
	}
private:
	task_completion_event<ComPtr<IAudioClient2>> completionEvent;
};
#endif

WASAPIMediaSink::WASAPIMediaSink(IMediaSinkHandler& handler)
	:IMediaSink(handler), sampleRequestEvent(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS)),
	mcssProvider(MFWorkerQueueProvider::GetProAudio().CreateSerial()), sinkHandler(handler)
{
	startThread = mcssProvider->QueueWorkerThread(
		std::bind(&WASAPIMediaSink::OnStart, this));
	seekThread = mcssProvider->QueueWorkerThread(
		std::bind(&WASAPIMediaSink::OnSeek, this));
	pauseThread = mcssProvider->QueueWorkerThread(
		std::bind(&WASAPIMediaSink::OnPause, this));
	stopThread = mcssProvider->QueueWorkerThread(
		std::bind(&WASAPIMediaSink::OnStop, this));
	sampleRequestedThread = mcssProvider->QueueWorkerThread(
		std::bind(&WASAPIMediaSink::OnSampleRequested, this));

	loadDeviceTask = InitializeDevice();
}

task<void> WASAPIMediaSink::InitializeDevice()
{
#if WINAPI_PARTITION_APP
	auto activateHandler = Make<ActivateAudioInterfaceCompletionHandler>();
	return activateHandler->Activate().then([=](ComPtr<IAudioClient2> audioClient)
	{
		this->audioClient = audioClient;
		ConfigureDevice();
	}, task_continuation_context::use_current());
#else
#error "Not support."
#endif
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

	// 初始化 AudioClient
	THROW_IF_FAILED(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
		0, 0, deviceInputFormat.get(), nullptr));
	// 获取设备缓冲大小
	THROW_IF_FAILED(audioClient->GetBufferSize(&deviceBufferFrames));
	// 获取音频渲染客户端
	THROW_IF_FAILED(audioClient->GetService(IID_PPV_ARGS(renderClient.ReleaseAndGetAddressOf())));
	// 获取简单音量控制
	THROW_IF_FAILED(audioClient->GetService(IID_PPV_ARGS(simpleAudioVolume.ReleaseAndGetAddressOf())));
	// Sets the event handle that the system signals when an audio buffer is ready to be processed by the client
	THROW_IF_FAILED(audioClient->SetEventHandle(sampleRequestEvent.Get()));

	InitializeDeviceBuffer();
}

void WASAPIMediaSink::SetMediaSourceReader(std::shared_ptr<ISourceReader> sourceReader)
{
	{
		auto lock(stateMonitor.Lock());

		// 状态必须为 Closed 或 Stopped
		if ((sinkState != MediaSinkStatus::Closed &&
			sinkState != MediaSinkStatus::Stopped) || seeking)
			THROW_IF_FAILED(E_NOT_VALID_STATE);

		SetStatus(MediaSinkStatus::Changing);
	}

	ReportOnError(loadDeviceTask.then([=]
	{
		setMediaSourceReaderThread = mcssProvider->QueueWorkerThread([=]
		{
			sourceReaderHolder = sourceReader;
			this->sourceReader = sourceReaderHolder.get();
			if (this->sourceReader)
				this->sourceReader->SetAudioFormat(deviceInputFormat.get(), GetBufferFramesPerPeriod());
			currentFrames = 0;

			{
				auto lock(stateMonitor.Lock());
				SetStatus(MediaSinkStatus::Paused);
			}
			sinkHandler.OnMediaOpened();
		});
		setMediaSourceReaderThread->Execute();
	}));
}

void WASAPIMediaSink::ReportOnError(concurrency::task<void> task)
{
	task.then([=](concurrency::task<void> t)
	{
		try
		{
			t.get();
		}
		catch (Exception^ ex)
		{
			sinkHandler.OnError(ex->HResult);
		}
		catch (...)
		{
			sinkHandler.OnError(E_UNEXPECTED);
		}
	});
}

void WASAPIMediaSink::Start()
{
	{
		auto lock(stateMonitor.Lock());

		// 状态必须为 Paused 或 Stopped
		if ((sinkState != MediaSinkStatus::Paused &&
			sinkState != MediaSinkStatus::Stopped) || seeking)
			THROW_IF_FAILED(E_NOT_VALID_STATE);

		if (GetPosition() == GetDuration())
			THROW_IF_FAILED(E_NOT_VALID_STATE);

		SetStatus(MediaSinkStatus::Changing);
	}

	startThread->Execute();
}

void WASAPIMediaSink::Pause()
{
	{
		auto lock(stateMonitor.Lock());
		// 状态必须为 Playing
		if ((sinkState != MediaSinkStatus::Started) || seeking)
			throw ref new Exception(E_NOT_VALID_STATE);

		SetStatus(MediaSinkStatus::Changing);
	}

	pauseThread->Execute();
}

void WASAPIMediaSink::Stop()
{
	{
		auto lock(stateMonitor.Lock());
		// 状态必须为 Started 或 Paused
		if ((sinkState != MediaSinkStatus::Started &&
			sinkState != MediaSinkStatus::Paused) || seeking)
			throw ref new Exception(E_NOT_VALID_STATE);

		SetStatus(MediaSinkStatus::Changing);
	}

	stopThread->Execute();
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

	if (isSilent || !sourceReader)
	{
		// 用空白填充缓冲
		byte* buffer = nullptr;
		THROW_IF_FAILED(renderClient->GetBuffer(framesAvailable, &buffer));
		THROW_IF_FAILED(renderClient->ReleaseBuffer(framesAvailable, AUDCLNT_BUFFERFLAGS_SILENT));
	}
	// Even if we cancel a work item, this may still fire due to the async
	// nature of things.  There should be a queued work item already to handle
	// the process of stopping or stopped
	else if (sinkState == MediaSinkStatus::Started)
		FillBufferFromMediaSource(framesAvailable);
}

void WASAPIMediaSink::FillBufferFromMediaSource(UINT32 framesCount)
{
	BYTE* data = nullptr;
	UINT32 actualBytesRead = 0;
	UINT32 actualBytesToRead = framesCount * deviceInputFormat->nBlockAlign;

	THROW_IF_FAILED(renderClient->GetBuffer(framesCount, &data));
	if (actualBytesRead = sourceReader->Read(data, actualBytesToRead))
	{
		auto framesRead = actualBytesRead / deviceInputFormat->nBlockAlign;
		currentFrames += framesRead;
		THROW_IF_FAILED(renderClient->ReleaseBuffer(framesRead, 0));
	}
	// 未读出数据
	else
	{
		THROW_IF_FAILED(renderClient->ReleaseBuffer(framesCount, AUDCLNT_BUFFERFLAGS_SILENT));
		// 媒体结束
		if (sourceReader->GetState() == SourceReaderState::Ended)
			OnMediaEnded();
	}
}

size_t WASAPIMediaSink::GetBufferFramesPerPeriod()
{
	REFERENCE_TIME defaultDevicePeriod = 0;
	REFERENCE_TIME minimumDevicePeriod = 0;

	// Get the audio device period
	THROW_IF_FAILED(audioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod));

	// 100 ns = 1e-7 s
	double devicePeriodInSeconds = hns2s(defaultDevicePeriod);

	return static_cast<UINT32>(deviceInputFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5);
}

void WASAPIMediaSink::InitializeDeviceBuffer()
{
	FillBufferAvailable(true);
}

void WASAPIMediaSink::OnMediaEnded()
{
	{
		auto lock(stateMonitor.Lock());
		// 状态必须为 Started
		if ((sinkState != MediaSinkStatus::Started) || seeking)
			throw ref new Exception(E_NOT_VALID_STATE);

		SetStatus(MediaSinkStatus::Paused);
		sinkHandler.OnMediaEnded();
	}
}

void WASAPIMediaSink::OnStart()
{
	//InitializeDeviceBuffer();
	THROW_IF_FAILED(audioClient->Start());

	if(sourceReader || sourceReader->GetState() == SourceReaderState::Stopped)
		sourceReader->Start(seekPosition);

	SetStatus(MediaSinkStatus::Started);

	sampleRequestedThread->Execute(sampleRequestEvent);
}

void WASAPIMediaSink::OnSeek()
{
	FillBufferAvailable(true);

	// 如果正在播放或暂停则 Seek 之后需要恢复
	if (sinkState == MediaSinkStatus::Started ||
		sinkState == MediaSinkStatus::Paused)
	{
		sourceReader->StopAsync().wait();
		sourceReader->Start(seekPosition);
	}

	seeking = false;
	sinkHandler.OnSeekCompleted();
}

void WASAPIMediaSink::OnPause()
{
	FillBufferAvailable(true);
	THROW_IF_FAILED(audioClient->Stop());

	SetStatus(MediaSinkStatus::Paused);
}

void WASAPIMediaSink::OnStop()
{
	FillBufferAvailable(true);
	THROW_IF_FAILED(audioClient->Stop());
	sourceReader->StopAsync().wait();

	seekPosition = 0;
	currentFrames = 0;

	SetStatus(MediaSinkStatus::Stopped);
}

void WASAPIMediaSink::OnSampleRequested()
{
	FillBufferAvailable(false);

	if (sinkState == MediaSinkStatus::Started)
	{
		// 安排下一次采样工作
		sampleRequestedThread->Execute(sampleRequestEvent);
	}
}

void WASAPIMediaSink::SetStatus(MediaSinkStatus status)
{
	auto lock(stateMonitor.Lock());
	if (sinkState != status)
	{
		sinkState = status;
		sinkHandler.OnStatusChanged(status);
	}
}

int64_t WASAPIMediaSink::GetPosition() const
{
	auto dt = (double)currentFrames / deviceInputFormat->nSamplesPerSec;
	return std::max(0ll, static_cast<int64_t>(dt * 1e7 +
		(sourceReader ? sourceReader->GetBufferStartPosition() : 0)));
}

void WASAPIMediaSink::SetPosition(int64_t position)
{
	{
		auto lock(stateMonitor.Lock());
		if (sinkState == MediaSinkStatus::Changing ||
			sinkState == MediaSinkStatus::Closed || seeking)
			THROW_IF_FAILED(E_NOT_VALID_STATE);

		if (position > GetDuration())
			THROW_IF_FAILED(E_INVALIDARG);

		if (GetPosition() != position)
		{
			seeking = true;
			seekPosition = position;
			seekThread->Execute();
		}
		else
		{
			sinkHandler.OnSeekCompleted();
		}
	}
}

int64_t WASAPIMediaSink::GetDuration() const
{
	return sourceReader ? sourceReader->GetDuration() : 0;
}

double WASAPIMediaSink::GetVolume()
{
	float volume;
	THROW_IF_FAILED(simpleAudioVolume->GetMasterVolume(&volume));
	return volume;
}

void WASAPIMediaSink::SetVolume(double value)
{
	value = std::min(1.0, std::max(0.0, value));

	THROW_IF_FAILED(simpleAudioVolume->SetMasterVolume(static_cast<float>(value), nullptr));
}

MEDIA_CORE_API std::unique_ptr<IMediaSink> __stdcall NS_TOMATO_MEDIA::CreateWASAPIMediaSink(IMediaSinkHandler& handler)
{
	return std::make_unique<WASAPIMediaSink>(handler);
}