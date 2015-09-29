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

BackgroundMediaPlayer::BackgroundMediaPlayer()
{

}

void BackgroundMediaPlayer::Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^taskInstance)
{
	AttachMessageHandlers();
	ConfigureMediaPlayer();

	deferral = taskInstance->GetDeferral();
	ActivateHandler();
	auto valueSet = ref new ValueSet();
	valueSet->Insert(L"MessageId", BackgroundMediaPlayerActivatedMessageKey);
	Playback::BackgroundMediaPlayer::SendMessageToForeground(valueSet);
}

void BackgroundMediaPlayer::ActivateHandler()
{
	auto audioHandlerName = GetResetSetting<settings::BackgroundMediaPlayerHandlerFullNameSetting>();

	Object^ handlerObj;
	ThrowIfFailed(RoActivateInstance(reinterpret_cast<HSTRING>(audioHandlerName),
		reinterpret_cast<IInspectable**>(&handlerObj)));
	auto factory = Make<MediaPlayerHandlerGetterFactory>(reinterpret_cast<IInspectable*>(handlerObj));

	//RO_REGISTRATION_COOKIE revoke;
	//auto classId = reinterpret_cast<HSTRING>(BackgroundMediaPlyaerHanderGetter::typeid->FullName);
	//PFNGETACTIVATIONFACTORY fn = GetFactories;
	//ThrowIfFailed(RoRegisterActivationFactories(&classId, &fn, 1, &revoke));

	auto audioHandler = safe_cast<IBackgroundMediaPlayerHandler^>(handlerObj);
	audioHandler->OnActivated(this);
}

void BackgroundMediaPlayer::AttachMessageHandlers()
{
	Playback::BackgroundMediaPlayer::MessageReceivedFromForeground += ref new EventHandler<Playback::MediaPlayerDataReceivedEventArgs ^>(
		this, &BackgroundMediaPlayer::OnMessageReceivedFromForeground);
}

void BackgroundMediaPlayer::ConfigureMediaPlayer()
{
	mediaPlayer = Playback::BackgroundMediaPlayer::Current;
	mediaPlayer->AudioCategory = Playback::MediaPlayerAudioCategory::Media;
	mediaPlayer->AudioDeviceType = Playback::MediaPlayerAudioDeviceType::Multimedia;
	mediaPlayer->AutoPlay = false;

	mediaPlayer->MediaOpened += ref new Windows::Foundation::TypedEventHandler<Playback::MediaPlayer ^, Object ^>(
		this, &BackgroundMediaPlayer::OnMediaOpened);
	mediaPlayer->CurrentStateChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Playback::MediaPlayer ^, Platform::Object ^>(this, &BackgroundMediaPlayer::OnCurrentStateChanged);
}


void BackgroundMediaPlayer::OnMessageReceivedFromForeground(Platform::Object ^sender, Playback::MediaPlayerDataReceivedEventArgs ^args)
{

}

void BackgroundMediaPlayer::SetMediaSource(MediaSource^ mediaSource)
{
	auto coreSource = Make<CoreMediaSource>(mediaSource->MFMediaSource);
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
