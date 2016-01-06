//
// Tomato Media
// 后台媒体播放器
// 
// 作者：SunnyCase 
// 创建日期 2015-08-04
#include "pch.h"
#include "BackgroundMediaPlayer.h"
#include "ApplicationDataHelper.h"

#define INITGUID
#include <guiddef.h>
#include "IBackgroundMediaPlayerHandler.h"
#include "CoreMediaSource.h"

using namespace Platform;
using namespace NS_MEDIA;
using namespace NS_MEDIA::details;
using namespace Windows::Media;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace WRL;

namespace
{
	class MediaPlayerHandlerGetterFactory : public ActivationFactory<>
	{
	public:
		MediaPlayerHandlerGetterFactory(IInspectable* handler)
			:_handler(handler)
		{

		}

		STDMETHOD(ActivateInstance)(_Outptr_result_nullonfailure_ IInspectable **ppvObject)
		{
			return _handler.CopyTo(ppvObject);
		}
	private:
		ComPtr<IInspectable> _handler;
	};

	HRESULT STDAPICALLTYPE GetFactories(HSTRING, IActivationFactory ** pFactory)
	{
		static Object^ obj = nullptr;
		static auto factory = Make<MediaPlayerHandlerGetterFactory>(reinterpret_cast<IInspectable*>(obj));
		return factory.CopyTo(pFactory);
	}
}

String^ ::NS_MEDIA::BackgroundMediaPlayerActivatedMessageKey = L"Tomato.Media.BackgroundMediaPlayer.Activated";
String^ ::NS_MEDIA::BackgroundMediaPlayerUserMessageKey = L"Tomato.Media.BackgroundMediaPlayer.UserMessage";

BackgroundMediaPlayer::BackgroundMediaPlayer()
{

}

void BackgroundMediaPlayer::Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance)
{
	AttachMessageHandlers();

	deferral = taskInstance->GetDeferral();

	ConfigureMediaPlayer();

	ActivateHandler();
	auto valueSet = ref new ValueSet();
	valueSet->Insert(L"MessageId", BackgroundMediaPlayerActivatedMessageKey);
	Playback::BackgroundMediaPlayer::SendMessageToForeground(valueSet);

	taskInstance->Task->Completed += ref new Windows::ApplicationModel::Background::BackgroundTaskCompletedEventHandler(this, &BackgroundMediaPlayer::OnCompleted);
	taskInstance->Canceled += ref new Windows::ApplicationModel::Background::BackgroundTaskCanceledEventHandler(this, &BackgroundMediaPlayer::OnCanceled);
}

void BackgroundMediaPlayer::SendMessage(String^ tag, String ^ message)
{
	auto valueSet = ref new ValueSet();
	valueSet->Insert(L"MessageId", BackgroundMediaPlayerUserMessageKey);
	valueSet->Insert(L"MessageTag", tag);
	valueSet->Insert(L"MessageContent", message);
	Playback::BackgroundMediaPlayer::SendMessageToForeground(valueSet);
}

void BackgroundMediaPlayer::ActivateHandler()
{
	auto audioHandlerName = GetResetSetting<settings::BackgroundMediaPlayerHandlerFullNameSetting>();

	Object^ handlerObj;
	ThrowIfFailed(RoActivateInstance(reinterpret_cast<HSTRING>(audioHandlerName),
		reinterpret_cast<IInspectable**>(&handlerObj)));
	auto audioHandler = safe_cast<IBackgroundMediaPlayerHandler^>(handlerObj);
	audioHandler->OnActivated(this);
	_audioHandler = audioHandler;
}

void BackgroundMediaPlayer::AttachMessageHandlers()
{
	_messageFromForegroundRegToken = Playback::BackgroundMediaPlayer::MessageReceivedFromForeground += ref new EventHandler<Playback::MediaPlayerDataReceivedEventArgs ^>(
		this, &BackgroundMediaPlayer::OnMessageReceivedFromForeground);
}

void BackgroundMediaPlayer::ConfigureMediaPlayer()
{
	mediaPlayer = Playback::BackgroundMediaPlayer::Current;
	mediaPlayer->AudioCategory = Playback::MediaPlayerAudioCategory::Media;
	mediaPlayer->AudioDeviceType = Playback::MediaPlayerAudioDeviceType::Multimedia;
	mediaPlayer->AutoPlay = false;

	_mediaOpenedRegToken = mediaPlayer->MediaOpened += ref new Windows::Foundation::TypedEventHandler<Playback::MediaPlayer ^, Object ^>(
		this, &BackgroundMediaPlayer::OnMediaOpened);
	_mediaEndedRegToken = mediaPlayer->MediaEnded += ref new Windows::Foundation::TypedEventHandler<Playback::MediaPlayer ^, Object ^>(
		this, &BackgroundMediaPlayer::OnMediaEnded);
	_mediaFailedRegToken = mediaPlayer->MediaFailed += ref new Windows::Foundation::TypedEventHandler<Playback::MediaPlayer ^, Playback::MediaPlayerFailedEventArgs ^>(this, 
		&BackgroundMediaPlayer::OnMediaFailed);
	_currentStateChangedRegToken = mediaPlayer->CurrentStateChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Platform::Object ^>(this, &BackgroundMediaPlayer::OnCurrentStateChanged);
	_seekCompletedRegToken = mediaPlayer->SeekCompleted += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Platform::Object ^>(this, &BackgroundMediaPlayer::OnSeekCompleted);
}

void BackgroundMediaPlayer::Shutdown()
{
	mediaPlayer->MediaOpened -= _mediaOpenedRegToken;
	mediaPlayer->MediaEnded -= _mediaEndedRegToken;
	mediaPlayer->MediaFailed -= _mediaFailedRegToken;
	mediaPlayer->CurrentStateChanged -= _currentStateChangedRegToken;
	mediaPlayer->SeekCompleted -= _seekCompletedRegToken;
	Playback::BackgroundMediaPlayer::MessageReceivedFromForeground -= _messageFromForegroundRegToken;
}

void BackgroundMediaPlayer::OnMessageReceivedFromForeground(Platform::Object ^sender, Playback::MediaPlayerDataReceivedEventArgs ^args)
{
	auto valueSet = args->Data;
	auto key = (String^)valueSet->Lookup(L"MessageId");
	if (key == BackgroundMediaPlayerUserMessageKey)
	{
		_audioHandler->OnReceiveMessage((String^)valueSet->Lookup(L"MessageTag"), (String^)valueSet->Lookup(L"MessageContent"));
	}
}

void BackgroundMediaPlayer::SetMediaSource(MediaSource^ mediaSource)
{
	auto coreSource = Make<CoreMediaSource>(mediaSource);
	mediaPlayer->SetMediaSource(reinterpret_cast<Windows::Media::Core::IMediaSource^>(coreSource.Get()));
}

void BackgroundMediaPlayer::Play()
{
	mediaPlayer->Play();
}

void BackgroundMediaPlayer::Pause()
{
	mediaPlayer->Pause();
}

void BackgroundMediaPlayer::OnMediaOpened(Playback::MediaPlayer ^sender, Platform::Object ^args)
{
	MediaOpened(this, args);
}

void BackgroundMediaPlayer::OnCurrentStateChanged(Playback::MediaPlayer ^sender, Platform::Object ^args)
{
	CurrentStateChanged(this, args);
}

void BackgroundMediaPlayer::OnMediaEnded(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args)
{
	MediaEnded(this, args);
}

void BackgroundMediaPlayer::OnMediaFailed(Windows::Media::Playback::MediaPlayer ^sender, Windows::Media::Playback::MediaPlayerFailedEventArgs ^args)
{
	MediaFailed(this, args);
}

void BackgroundMediaPlayer::OnSeekCompleted(Windows::Media::Playback::MediaPlayer ^sender, Platform::Object ^args)
{
	SeekCompleted(this, args);
}

void BackgroundMediaPlayer::OnCompleted(Windows::ApplicationModel::Background::BackgroundTaskRegistration ^sender, Windows::ApplicationModel::Background::BackgroundTaskCompletedEventArgs ^args)
{
	deferral->Complete();
}

void BackgroundMediaPlayer::OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason)
{
	auto fin(make_finalizer([&] {deferral->Complete();}));
	Shutdown();
	_audioHandler->OnCanceled();
}
