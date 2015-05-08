//
// Tomato Media
// 后台音频任务
// 
// (c) SunnyCase 
// 创建日期 2015-05-06
#include "pch.h"
#include "BackgroundAudioTask.h"
#include "MediaSource.h"

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP

using namespace Tomato::Media;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Media;
using namespace Windows::Media::Playback;
using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace concurrency;

namespace
{
	struct AudioControlMessageKeys
	{
		static String^ Message;
		static String^ MessageId;
		static String^ Exception;
		static String^ Value;
	};

	String^ AudioControlMessageKeys::Message = L"Message";
	String^ AudioControlMessageKeys::MessageId = L"MessageId";
	String^ AudioControlMessageKeys::Exception = L"Exception";
	String^ AudioControlMessageKeys::Value = L"Value";

	struct AudioControlMessages
	{
		static String^ Initialize;
		static String^ SetMediaSource;
		static String^ StartPlayback;
		static String^ SeekPlayback;
		static String^ PausePlayback;
		static String^ StopPlayback;
		static String^ GetIsPlayEnabled;
		static String^ SetIsPlayEnabled;
		static String^ GetIsPauseEnabled;
		static String^ SetIsPauseEnabled;
		static String^ GetIsNextEnabled;
		static String^ SetIsNextEnabled;
		static String^ GetIsPreviousEnabled;
		static String^ SetIsPreviousEnabled;
		static String^ GetIsSystemMediaControlEnabled;
		static String^ SetIsSystemMediaControlEnabled;
	};

	String^ AudioControlMessages::Initialize = L"Initialize";
	String^ AudioControlMessages::SetMediaSource = L"SetMediaSource";
	String^ AudioControlMessages::StartPlayback = L"StartPlayback";
	String^ AudioControlMessages::SeekPlayback = L"SeekPlayback";
	String^ AudioControlMessages::PausePlayback = L"PausePlayback";
	String^ AudioControlMessages::StopPlayback = L"StopPlayback";
	String^ AudioControlMessages::GetIsPlayEnabled = L"GetIsPlayEnabled";
	String^ AudioControlMessages::SetIsPlayEnabled = L"SetIsPlayEnabled";
	String^ AudioControlMessages::GetIsPauseEnabled = L"GetIsPauseEnabled";
	String^ AudioControlMessages::SetIsPauseEnabled = L"SetIsPauseEnabled";
	String^ AudioControlMessages::GetIsNextEnabled = L"GetIsNextEnabled";
	String^ AudioControlMessages::SetIsNextEnabled = L"SetIsNextEnabled";
	String^ AudioControlMessages::GetIsPreviousEnabled = L"GetIsPreviousEnabled";
	String^ AudioControlMessages::SetIsPreviousEnabled = L"SetIsPreviousEnabled";
	String^ AudioControlMessages::GetIsSystemMediaControlEnabled = L"GetIsSystemMediaControlEnabled";
	String^ AudioControlMessages::SetIsSystemMediaControlEnabled = L"SetIsSystemMediaControlEnabled";

	uint64_t AcquiredMessageId()
	{
		static LONGLONG messageId = 1;
		return static_cast<uint64_t>(InterlockedIncrement64(&messageId));
	}

	void SendMessageToBackground(String^ message, ValueSet^ valueSet)
	{
		valueSet->Insert(AudioControlMessageKeys::Message, message);

		BackgroundMediaPlayer::SendMessageToBackground(valueSet);
	}

	void SendMessageToBackground(String^ message)
	{
		auto valueSet = ref new ValueSet();
		valueSet->Insert(AudioControlMessageKeys::Message, message);

		BackgroundMediaPlayer::SendMessageToBackground(valueSet);
	}

	void SendMessageToBackground(String^ message, uint64_t messageId)
	{
		auto valueSet = ref new ValueSet();
		valueSet->Insert(AudioControlMessageKeys::Message, message);
		valueSet->Insert(AudioControlMessageKeys::MessageId, messageId);

		BackgroundMediaPlayer::SendMessageToBackground(valueSet);
	}

	void SendMessageToBackground(String^ message, uint64_t messageId, ValueSet^ valueSet)
	{
		valueSet->Insert(AudioControlMessageKeys::Message, message);
		valueSet->Insert(AudioControlMessageKeys::MessageId, messageId);

		BackgroundMediaPlayer::SendMessageToBackground(valueSet);
	}

	ref class BackgroundMessageHandler sealed
	{
	public:
		property static BackgroundMessageHandler^ Current
		{
			BackgroundMessageHandler^ get()
			{
				static auto handler = ref new BackgroundMessageHandler();
				return handler;
			}
		}
	internal:
		void WaitForMessage(uint64_t messageId, std::function<void(ValueSet^)> callback)
		{
			std::lock_guard<decltype(callbacksMutex)> locker(callbacksMutex);

			callbacks[messageId] = std::move(callback);
		}

		void SetStatusCallback(std::function<void(ValueSet^)> callback)
		{
			statusCallback = std::move(callback);
		}
	private:
		BackgroundMessageHandler()
		{
			BackgroundMediaPlayer::MessageReceivedFromBackground +=
				ref new EventHandler<MediaPlayerDataReceivedEventArgs ^>(this, &BackgroundMessageHandler::OnMessageReceivedFromBackground);
		}

		void OnMessageReceivedFromBackground(Object ^sender, MediaPlayerDataReceivedEventArgs ^args)
		{
			if (args->Data->HasKey(AudioControlMessageKeys::MessageId))
			{
				auto messageId = static_cast<uint64_t>(args->Data->Lookup(AudioControlMessageKeys::MessageId));

				std::function<void(ValueSet^)> callback;
				{
					// 检查是否注册了回调
					std::lock_guard<decltype(callbacksMutex)> locker(callbacksMutex);
					auto it = callbacks.find(messageId);
					if (it != callbacks.end())
					{
						callback = std::move(it->second);
						callbacks.erase(it);
					}
				}
				if (callback)
					callback(args->Data);
			}
			// 其他状态消息
			else if (statusCallback)
				statusCallback(args->Data);
		}
	private:
		std::unordered_map<uint64_t, std::function<void(ValueSet^)>> callbacks;
		std::function<void(ValueSet^)> statusCallback;
		std::mutex callbacksMutex;
	};

	task<void> SendMessageToBackgroundAndWait(String^ message, uint64_t messageId = AcquiredMessageId())
	{
		task_completion_event<void> completion;
		BackgroundMessageHandler::Current->WaitForMessage(messageId, [=](ValueSet^ value)
		{
			if (value->HasKey(AudioControlMessageKeys::Exception))
				completion.set_exception(ref new Exception(E_FAIL, static_cast<String^>(
					value->Lookup(AudioControlMessageKeys::Exception))));
			else
				completion.set();
		});
		SendMessageToBackground(message, messageId);
		return create_task(completion);
	}

	task<void> SendMessageToBackgroundAndWait(String^ message, ValueSet^ valueSet, uint64_t messageId = AcquiredMessageId())
	{
		task_completion_event<void> completion;
		BackgroundMessageHandler::Current->WaitForMessage(messageId, [=](ValueSet^ value)
		{
			if (value->HasKey(AudioControlMessageKeys::Exception))
				completion.set_exception(ref new Exception(E_FAIL, static_cast<String^>(
					value->Lookup(AudioControlMessageKeys::Exception))));
			else
				completion.set();
		});
		SendMessageToBackground(message, messageId, valueSet);
		return create_task(completion);
	}

	template<typename T>
	task<T> SendMessageToBackgroundAndWait(String^ message, uint64_t messageId = AcquiredMessageId())
	{
		task_completion_event<T> completion;
		BackgroundMessageHandler::Current->WaitForMessage(messageId, [=](ValueSet^ value)
		{
			if (value->HasKey(AudioControlMessageKeys::Exception))
				completion.set_exception(ref new Exception(E_FAIL, static_cast<String^>(
					value->Lookup(AudioControlMessageKeys::Exception))));
			else
			{
				auto data = static_cast<T>(value->Lookup(AudioControlMessageKeys::Value));
				completion.set(data);
			}
		});
		SendMessageToBackground(message, messageId);
		return create_task(completion);
	}

	struct AudioStatusMessages
	{
		static String^ StatusChanged;
		static String^ PlayButtonPressed;
		static String^ PauseButtonPressed;
		static String^ StopButtonPressed;
		static String^ PreviousButtonPressed;
		static String^ NextButtonPressed;
		static String^ MediaEnded;
	};

	String^ AudioStatusMessages::StatusChanged = L"StatusChanged";
	String^ AudioStatusMessages::PlayButtonPressed = L"PlayButtonPressed";
	String^ AudioStatusMessages::PauseButtonPressed = L"PauseButtonPressed";
	String^ AudioStatusMessages::StopButtonPressed = L"StopButtonPressed";
	String^ AudioStatusMessages::PreviousButtonPressed = L"PreviousButtonPressed";
	String^ AudioStatusMessages::NextButtonPressed = L"NextButtonPressed";
	String^ AudioStatusMessages::MediaEnded = L"MediaEnded";
}

bool BackgroundAudioTask::initialized = false;
CoreDispatcher^ BackgroundAudioTask::uiDispatcher = nullptr;
// 初始化消息 Id
#define INITIALIZE_MESSAGEID 0

BackgroundAudioTask::BackgroundAudioTask()
{
	RegisterMFTs();
}

void BackgroundAudioTask::Run(IBackgroundTaskInstance ^taskInstance)
{
	OutputDebugString(L"Run Audio Task.\n");

	BackgroundMediaPlayer::MessageReceivedFromForeground +=
		ref new EventHandler<MediaPlayerDataReceivedEventArgs ^>(this, &BackgroundAudioTask::OnMessageReceivedFromForeground);

	OnInitialize();
	Deferral = taskInstance->GetDeferral();
}

IAsyncAction ^ BackgroundAudioTask::Initialize(CoreDispatcher^ uiDispatcher)
{
	BackgroundAudioTask::uiDispatcher = uiDispatcher;

	return create_async([] {
		if (!initialized)
		{
			// 设置状态消息处理函数
			BackgroundMessageHandler::Current->SetStatusCallback([](ValueSet^ valueSet)
			{
				if (valueSet->HasKey(AudioControlMessageKeys::Message))
				{
					auto message = static_cast<String^>(valueSet->Lookup(AudioControlMessageKeys::Message));
					if (message == AudioStatusMessages::StatusChanged)
					{
						auto value = static_cast<MediaPlaybackStatus>(
							static_cast<int>(valueSet->Lookup(AudioControlMessageKeys::Value)));
						RunOnUIDispatcher([=] {
							OnMediaPlaybackStatusChanged(value);});
					}
					else if (message == AudioStatusMessages::PlayButtonPressed)
						PlayButtonPressed(nullptr, nullptr);
					else if (message == AudioStatusMessages::PauseButtonPressed)
						PauseButtonPressed(nullptr, nullptr);
					else if (message == AudioStatusMessages::StopButtonPressed)
						StopButtonPressed(nullptr, nullptr);
					else if (message == AudioStatusMessages::PreviousButtonPressed)
						PreviousButtonPressed(nullptr, nullptr);
					else if (message == AudioStatusMessages::NextButtonPressed)
						NextButtonPressed(nullptr, nullptr);
					else if (message == AudioStatusMessages::MediaEnded)
					{
						RunOnUIDispatcher([=] {
							MediaEnded(nullptr, nullptr);});
					}
				}
			});

			return SendMessageToBackgroundAndWait(AudioControlMessages::Initialize,
				INITIALIZE_MESSAGEID).then([&]
			{
				initialized = true;
			});
		}
		else
			return task_from_result();
	});
}

IAsyncAction^ BackgroundAudioTask::SetMediaSource(String^ path)
{
	EnsureInitialized();

	auto value = ref new ValueSet();
	value->Insert(AudioControlMessageKeys::Value, path);

	return create_async([=] {
		return SendMessageToBackgroundAndWait(AudioControlMessages::SetMediaSource, value);
	});
}

void BackgroundAudioTask::StartPlayback()
{
	EnsureInitialized();

	SendMessageToBackground(AudioControlMessages::StartPlayback);
}

void BackgroundAudioTask::StartPlayback(TimeSpan time)
{
	EnsureInitialized();

	auto value = ref new ValueSet();
	value->Insert(AudioControlMessageKeys::Value, time);
	SendMessageToBackground(AudioControlMessages::SeekPlayback, value);
}

void BackgroundAudioTask::PausePlayback()
{
	EnsureInitialized();

	SendMessageToBackground(AudioControlMessages::PausePlayback);
}

void BackgroundAudioTask::StopPlayback()
{
	EnsureInitialized();

	SendMessageToBackground(AudioControlMessages::StopPlayback);
}

void BackgroundAudioTask::OnInitialize()
{
	OutputDebugString(L"Initialize Audio Task.\n");

	try
	{
		mediaPlayer = BackgroundMediaPlayer::Current;
		mediaTransportControls = SystemMediaTransportControls::GetForCurrentView();
		mediaPlayer->MediaOpened += ref new TypedEventHandler<MediaPlayer ^, Platform::Object ^>(
			this, &BackgroundAudioTask::OnMediaOpened);
		mediaPlayer->MediaFailed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Windows::Media::Playback::MediaPlayerFailedEventArgs ^>(this, &Tomato::Media::BackgroundAudioTask::OnMediaFailed);
		mediaPlayer->MediaEnded += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Platform::Object ^>(this, &Tomato::Media::BackgroundAudioTask::OnMediaEnded);
		mediaPlayer->CurrentStateChanged += ref new TypedEventHandler<MediaPlayer ^, Object ^>(
			this, &BackgroundAudioTask::OnCurrentStateChanged);
		mediaPlayer->SeekCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Platform::Object ^>(this, &Tomato::Media::BackgroundAudioTask::OnSeekCompleted);

		InitializeMediaTransportControls();

		mediaPlayer->AutoPlay = false;

		ReplyMessageToForeground(AudioControlMessages::Initialize, INITIALIZE_MESSAGEID);
	}
	catch (Exception^ ex)
	{
		ReplyErrorToForeground(ex->Message, INITIALIZE_MESSAGEID);
	}
	catch (...)
	{
		ReplyErrorToForeground(L"Unknown Exception.", INITIALIZE_MESSAGEID);
	}
}

void BackgroundAudioTask::InitializeMediaTransportControls()
{
	mediaTransportControls->ButtonPressed += ref new TypedEventHandler<SystemMediaTransportControls ^,
		SystemMediaTransportControlsButtonPressedEventArgs ^>(this, &BackgroundAudioTask::OnButtonPressed);
	mediaTransportControls->IsStopEnabled = false;

	mediaTransportControls->PlaybackStatus = MediaPlaybackStatus::Closed;
	mediaTransportControls->DisplayUpdater->Type = MediaPlaybackType::Music;
	mediaTransportControls->DisplayUpdater->Update();
}

void BackgroundAudioTask::OnButtonPressed(SystemMediaTransportControls ^sender,
	SystemMediaTransportControlsButtonPressedEventArgs ^args)
{
	auto button = args->Button;

	switch (args->Button)
	{
	case SystemMediaTransportControlsButton::Play:
		SendStatusMessageToForeground(AudioStatusMessages::PlayButtonPressed);
		break;
	case SystemMediaTransportControlsButton::Pause:
		SendStatusMessageToForeground(AudioStatusMessages::PauseButtonPressed);
		break;
	case SystemMediaTransportControlsButton::Stop:
		SendStatusMessageToForeground(AudioStatusMessages::StopButtonPressed);
		break;
	case SystemMediaTransportControlsButton::Previous:
		SendStatusMessageToForeground(AudioStatusMessages::PreviousButtonPressed);
		break;
	case SystemMediaTransportControlsButton::Next:
		SendStatusMessageToForeground(AudioStatusMessages::NextButtonPressed);
		break;
	default:
		break;
	}
}

void BackgroundAudioTask::RunOnUIDispatcher(std::function<void()>&& handler)
{
	if (uiDispatcher)
		uiDispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(std::move(handler)));
	else
		handler();
}

void BackgroundAudioTask::OnSetMediaSource(ValueSet ^ value)
{
	auto path = static_cast<String^>(value->Lookup(AudioControlMessageKeys::Value));
	auto messageId = static_cast<uint64_t>(value->Lookup(AudioControlMessageKeys::MessageId));
	mediaOpenedEvent = decltype(mediaOpenedEvent)();

	ReplyErrorToForeground(create_task(StorageFile::GetFileFromPathAsync(path))
		.then([=](StorageFile^ file)
	{
		mediaPlayer->SetUriSource(ref new Uri(path));

		return create_task(MediaSource::CreateFromFile(file)).then([=](MediaSource^ source)
		{
			RunOnUIDispatcher([=]
			{
				auto updater = mediaTransportControls->DisplayUpdater;

				updater->MusicProperties->Title = source->Title;
				updater->MusicProperties->AlbumArtist = source->AlbumArtist;
				updater->MusicProperties->Artist = source->Artist;

				mediaTransportControls->PlaybackStatus = MediaPlaybackStatus::Closed;
				updater->Update();
			});

			return create_task(mediaOpenedEvent).then([=] {
				ReplyMessageToForeground(AudioControlMessages::SetMediaSource, messageId);
			});
		});
	}), messageId);
}

void BackgroundAudioTask::OnStartPlayback()
{
	if (mediaPlayer->CurrentState != MediaPlayerState::Playing)
		mediaPlayer->Play();
}

void BackgroundAudioTask::OnSeekPlayback(ValueSet ^ value)
{
	if (mediaPlayer->CanSeek)
	{
		auto position = static_cast<TimeSpan>(value->Lookup(AudioControlMessageKeys::Value));
		mediaPlayer->Position = position;
		if (mediaPlayer->CurrentState == MediaPlayerState::Paused ||
			mediaPlayer->CurrentState == MediaPlayerState::Stopped)
			mediaPlayer->Play();
	}
}

void BackgroundAudioTask::OnPausePlayback()
{
	mediaPlayer->Pause();
}

void BackgroundAudioTask::OnStopPlayback()
{
	mediaPlayer->Pause();
	mediaPlayer->Position = TimeSpan{ 0 };
}

void BackgroundAudioTask::ReplyMessageToForeground(String ^ message, uint64_t messageId)
{
	auto valueSet = ref new ValueSet();
	valueSet->Insert(AudioControlMessageKeys::Message, message);
	valueSet->Insert(AudioControlMessageKeys::MessageId, messageId);

	BackgroundMediaPlayer::SendMessageToForeground(valueSet);
}

void BackgroundAudioTask::ReplyMessageToForeground(String^ message, uint64_t messageId, ValueSet^ valueSet)
{
	valueSet->Insert(AudioControlMessageKeys::Message, message);
	valueSet->Insert(AudioControlMessageKeys::MessageId, messageId);

	BackgroundMediaPlayer::SendMessageToForeground(valueSet);
}

void BackgroundAudioTask::ReplyErrorToForeground(String ^ errorMessage, uint64_t messageId)
{
	auto valueSet = ref new ValueSet();
	valueSet->Insert(AudioControlMessageKeys::MessageId, messageId);
	valueSet->Insert(AudioControlMessageKeys::Exception, errorMessage);

	BackgroundMediaPlayer::SendMessageToForeground(valueSet);
}

void BackgroundAudioTask::ReplyErrorToForeground(task<void> t, uint64_t messageId)
{
	t.then([=](task<void> t)
	{
		try
		{
			t.get();
		}
		catch (Exception^ ex)
		{
			ReplyErrorToForeground(ex->Message, messageId);
		}
		catch (...)
		{
			ReplyErrorToForeground(L"Unknown Exception.", messageId);
		}
	});
}

void BackgroundAudioTask::EnsureInitialized()
{
	if (!initialized)
		throw ref new Exception(E_NOT_VALID_STATE);
}

void BackgroundAudioTask::OnMediaPlaybackStatusChanged(MediaPlaybackStatus status)
{
	MediaPlaybackStatusChanged(nullptr, status);
}

#define DEFINE_MEDIA_TRANS_BOOL_ACCESSER_HANDLER(name) else if (message == AudioControlMessages::Get##name)			\
{																													\
	auto messageId = static_cast<uint64_t>(args->Data->Lookup(AudioControlMessageKeys::MessageId));					\
	auto value = mediaTransportControls->##name;																		\
	auto valueSet = ref new ValueSet();																				\
	valueSet->Insert(AudioControlMessageKeys::Value, value);															\
																													\
	ReplyMessageToForeground(message, messageId, valueSet);															\
}																													\
else if (message == AudioControlMessages::Set##name)																\
{																													\
	auto value = static_cast<bool>(args->Data->Lookup(AudioControlMessageKeys::Value));								\
	mediaTransportControls->##name = value;																			\
	mediaTransportControls->DisplayUpdater->Update();																\
}

void BackgroundAudioTask::OnMessageReceivedFromForeground(Object ^sender, MediaPlayerDataReceivedEventArgs ^args)
{
	if (args->Data->HasKey(AudioControlMessageKeys::Message))
	{
		auto message = static_cast<String^>(args->Data->Lookup(AudioControlMessageKeys::Message));

		// Initialize 消息不做处理
		// if (message == AudioControlMessages::Initialize)
		if (message == AudioControlMessages::SetMediaSource)
			OnSetMediaSource(args->Data);
		else if (message == AudioControlMessages::StartPlayback)
			OnStartPlayback();
		else if (message == AudioControlMessages::SeekPlayback)
			OnSeekPlayback(args->Data);
		else if (message == AudioControlMessages::PausePlayback)
			OnPausePlayback();
		else if (message == AudioControlMessages::StopPlayback)
			OnStopPlayback();
		DEFINE_MEDIA_TRANS_BOOL_ACCESSER_HANDLER(IsPlayEnabled)
			DEFINE_MEDIA_TRANS_BOOL_ACCESSER_HANDLER(IsPauseEnabled)
			DEFINE_MEDIA_TRANS_BOOL_ACCESSER_HANDLER(IsNextEnabled)
		else if (message == AudioControlMessages::GetIsSystemMediaControlEnabled)
		{
			auto messageId = static_cast<uint64_t>(args->Data->Lookup(AudioControlMessageKeys::MessageId));
			auto value = mediaTransportControls->IsEnabled;
			auto valueSet = ref new ValueSet();
			valueSet->Insert(AudioControlMessageKeys::Value, value);

			ReplyMessageToForeground(message, messageId, valueSet);
		}
		else if (message == AudioControlMessages::SetIsSystemMediaControlEnabled)
		{
			auto value = static_cast<bool>(args->Data->Lookup(AudioControlMessageKeys::Value));
			mediaTransportControls->IsEnabled = value;
			mediaTransportControls->DisplayUpdater->Update();
		}
	}
}

void BackgroundAudioTask::SendStatusMessageToForeground(String ^ message, ValueSet ^ valueSet)
{
	valueSet->Insert(AudioControlMessageKeys::Message, message);

	BackgroundMediaPlayer::SendMessageToForeground(valueSet);
}

void BackgroundAudioTask::SendStatusMessageToForeground(String ^ message)
{
	auto valueSet = ref new ValueSet();
	valueSet->Insert(AudioControlMessageKeys::Message, message);

	BackgroundMediaPlayer::SendMessageToForeground(valueSet);
}

void BackgroundAudioTask::OnMediaOpened(MediaPlayer ^sender, Object ^args)
{
	mediaOpenedEvent.set();
}

void BackgroundAudioTask::OnCurrentStateChanged(MediaPlayer ^sender, Object ^args)
{
	MediaPlaybackStatus playbackStatus;

	auto state = sender->CurrentState;
	switch (state)
	{
	case MediaPlayerState::Closed:
		playbackStatus = MediaPlaybackStatus::Closed;
		break;
	case MediaPlayerState::Opening:
		playbackStatus = MediaPlaybackStatus::Changing;
		break;
	case MediaPlayerState::Buffering:
		playbackStatus = MediaPlaybackStatus::Changing;
		break;
	case MediaPlayerState::Playing:
		playbackStatus = MediaPlaybackStatus::Playing;
		break;
	case MediaPlayerState::Paused:
		playbackStatus = MediaPlaybackStatus::Paused;
		break;
	case MediaPlayerState::Stopped:
		playbackStatus = MediaPlaybackStatus::Stopped;
		break;
	default:
		return;
	}
	auto valueSet = ref new ValueSet();

	RunOnUIDispatcher([=]
	{
		mediaTransportControls->PlaybackStatus = playbackStatus;
		mediaTransportControls->DisplayUpdater->Update();
	});
	valueSet->Insert(AudioControlMessageKeys::Value, static_cast<int>(playbackStatus));
	SendStatusMessageToForeground(AudioStatusMessages::StatusChanged, valueSet);
}

void BackgroundAudioTask::OnMediaFailed(MediaPlayer ^sender, MediaPlayerFailedEventArgs ^args)
{
	mediaOpenedEvent.set_exception(ref new Exception(args->ExtendedErrorCode.Value, args->ErrorMessage));
}

bool BackgroundAudioTask::IsPlayEnabled::get()
{
	return SendMessageToBackgroundAndWait<bool>(AudioControlMessages::GetIsPlayEnabled).get();
}

void BackgroundAudioTask::IsPlayEnabled::set(bool value)
{
	auto valueSet = ref new ValueSet();

	valueSet->Insert(AudioControlMessageKeys::Value, value);
	SendMessageToBackground(AudioControlMessages::SetIsPlayEnabled, valueSet);
}

bool BackgroundAudioTask::IsPauseEnabled::get()
{
	return SendMessageToBackgroundAndWait<bool>(AudioControlMessages::GetIsPauseEnabled).get();
}

void BackgroundAudioTask::IsPauseEnabled::set(bool value)
{
	auto valueSet = ref new ValueSet();

	valueSet->Insert(AudioControlMessageKeys::Value, value);
	SendMessageToBackground(AudioControlMessages::SetIsPauseEnabled, valueSet);
}

bool BackgroundAudioTask::IsNextEnabled::get()
{
	return SendMessageToBackgroundAndWait<bool>(AudioControlMessages::GetIsNextEnabled).get();
}

void BackgroundAudioTask::IsNextEnabled::set(bool value)
{
	auto valueSet = ref new ValueSet();

	valueSet->Insert(AudioControlMessageKeys::Value, value);
	SendMessageToBackground(AudioControlMessages::SetIsNextEnabled, valueSet);
}

bool BackgroundAudioTask::IsPreviousEnabled::get()
{
	return SendMessageToBackgroundAndWait<bool>(AudioControlMessages::GetIsPreviousEnabled).get();
}

void BackgroundAudioTask::IsPreviousEnabled::set(bool value)
{
	auto valueSet = ref new ValueSet();

	valueSet->Insert(AudioControlMessageKeys::Value, value);
	SendMessageToBackground(AudioControlMessages::SetIsPreviousEnabled, valueSet);
}

bool BackgroundAudioTask::IsSystemMediaControlEnabled::get()
{
	return SendMessageToBackgroundAndWait<bool>(AudioControlMessages::GetIsSystemMediaControlEnabled).get();
}

void BackgroundAudioTask::IsSystemMediaControlEnabled::set(bool value)
{
	auto valueSet = ref new ValueSet();

	valueSet->Insert(AudioControlMessageKeys::Value, value);
	SendMessageToBackground(AudioControlMessages::SetIsSystemMediaControlEnabled, valueSet);
}

TimeSpan BackgroundAudioTask::CurrentTime::get()
{
	return BackgroundMediaPlayer::Current->Position;
}

void BackgroundAudioTask::OnMediaEnded(MediaPlayer ^sender, Object ^args)
{
	SendStatusMessageToForeground(AudioStatusMessages::MediaEnded);
}

void BackgroundAudioTask::OnSeekCompleted(MediaPlayer ^sender, Object ^args)
{
	RunOnUIDispatcher([=]
	{
		mediaTransportControls->PlaybackStatus = MediaPlaybackStatus::Playing;
		mediaTransportControls->DisplayUpdater->Update();
	});
}

#endif

