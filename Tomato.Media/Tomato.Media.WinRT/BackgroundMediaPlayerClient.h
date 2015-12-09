//
// Tomato Media
// 后台媒体播放器客户端
// 
// 作者：SunnyCase 
// 创建日期 2015-08-04
#pragma once
#include "common.h"

DEFINE_NS_MEDIA

///<summary>后台音频播放器</summary>
public ref class BackgroundMediaPlayerClient sealed
{
public:
	///<summary>创建 <see cref="BackgroundMediaPlayerClient"/> 的新实例。</summary>
	///<param name="mediaPlayerHandlerTypeName">实现了 <see cref="IBackgroundMediaHandler"/> 的类型。</param>
	BackgroundMediaPlayerClient(Platform::String^ mediaPlayerHandlerTypeName);

	event Windows::Foundation::EventHandler<Platform::Object^>^ PlayerActivated;
	event Windows::Foundation::EventHandler<Platform::String^>^ MessageReceived;

	void SendMessage(Platform::String^ message);
private:
	void AttachMessageListener();
	void DetachMessageListener();

	void OnMessageReceivedFromBackground(Platform::Object^ sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs^ e);
private:
	Windows::Foundation::EventRegistrationToken messageListenerToken;
};

END_NS_MEDIA