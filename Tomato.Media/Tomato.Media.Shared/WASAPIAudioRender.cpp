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
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && __cplusplus_winrt
	using namespace Windows::Media::Devices;

	struct ActivateAudioInterfaceCompletionHandler : public RuntimeClass<RuntimeClassFlags<
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
			ThrowIfFailed(ActivateAudioInterfaceAsync(deviceId->Data(),
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
				ThrowIfFailed(activateOperation->GetActivateResult(&hr, &audioClientUnk));
				ThrowIfFailed(hr);

				ComPtr<IAudioClient2> audioClient;
				ThrowIfFailed(audioClientUnk.As(&audioClient));
				completionEvent.set(audioClient);
			}
			catch (Platform::Exception^ ex)
			{
				completionEvent.set_exception(ex);
				return ex->HResult;
			}
			catch (...)
			{
				completionEvent.set_exception(ref new Platform::COMException(E_FAIL));
				return E_FAIL;
			}
			return S_OK;
		}
	private:
		task_completion_event<ComPtr<IAudioClient2>> completionEvent;
	};

	task<ComPtr<IAudioClient2>> CreateAudioClient()
	{
		auto handler = Make<ActivateAudioInterfaceCompletionHandler>();
		return handler->Activate();
	}
#else
	auto CreateAudioClient()
	{
		ComPtr<IMMDeviceEnumerator> mmDeviceEnumerator;
		ThrowIfFailed(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
			IID_PPV_ARGS(&mmDeviceEnumerator)));
		ComPtr<IMMDevice> mmDevice;
		ThrowIfFailed(mmDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &mmDevice));
		ComPtr<IAudioClient> audioClient;
		ThrowIfFailed(mmDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, &audioClient));
		return task_from_result(audioClient);
	}
#endif
}

WASAPIAudioRender::WASAPIAudioRender()
	:sampleRequestEvent(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS))
{
	ThrowWin32IfNot(sampleRequestEvent.IsValid());
}

task<void> WASAPIAudioRender::Initialize()
{
	if (!initStarted.exchange(true))
		initTask = InitializeCore();
	return initTask;
}

task<void> WASAPIAudioRender::InitializeCore()
{
	return CreateDeviceResources();
}

task<void> WASAPIAudioRender::CreateDeviceResources()
{
	ComPtr<WASAPIAudioRender> thisGuard(this);
	return CreateAudioClient().then([thisGuard, this]
#if (WINVER >= _WIN32_WINNT_WIN8)
		(ComPtr<IAudioClient2> audioClient)
#elif (WINVER >= _WIN32_WINNT_VISTA)
		(ComPtr<IAudioClient> audioClient)
#endif
	{
		this->audioClient = audioClient;
		ConfigureDevice();
	});
}

void WASAPIAudioRender::ConfigureDevice()
{
	if (!audioClient) ThrowIfFailed(E_NOT_VALID_STATE);

#if (WINVER >= _WIN32_WINNT_WIN8)
	AudioClientProperties audioProps = { 0 };
	audioProps.cbSize = sizeof(audioProps);
	audioProps.bIsOffload = FALSE;
#if (NTDDI_VERSION >= NTDDI_WIN10)
	audioProps.eCategory = AudioCategory_Media;
#else
	audioProps.eCategory = AudioCategory_BackgroundCapableMedia;
#endif
	ThrowIfFailed(audioClient->SetClientProperties(&audioProps));
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
