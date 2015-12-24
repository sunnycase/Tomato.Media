//
// Tomato Media
// 后台媒体播放器客户端
// 
// 作者：SunnyCase 
// 创建日期 2015-08-04
#pragma once
#include "common.h"

DEFINE_NS_MEDIA

public ref class MessageReceivedEventArgs sealed
{
public:
	MessageReceivedEventArgs(Platform::String^ tag, Platform::String^ message)
		:tag(tag), message(message)
	{

	}

	property Platform::String^ Tag
	{
		Platform::String^ get() { return tag; }
	}

	property Platform::String^ Message
	{
		Platform::String^ get() { return message; }
	}
private:
	Platform::String^ tag;
	Platform::String^ message;
};

///<summary>后台音频播放器</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class BackgroundMediaPlayerClient sealed
{
public:
	///<summary>创建 <see cref="BackgroundMediaPlayerClient"/> 的新实例。</summary>
	///<param name="mediaPlayerHandlerTypeName">实现了 <see cref="IBackgroundMediaHandler"/> 的类型。</param>
	BackgroundMediaPlayerClient(Windows::UI::Xaml::Interop::TypeName mediaPlayerHandlerTypeName);

	event Windows::Foundation::EventHandler<Platform::Object^>^ PlayerActivated;
	event Windows::Foundation::EventHandler<MessageReceivedEventArgs^>^ MessageReceived;

	void SendMessage(Platform::String^ tag, Platform::String^ message);
private:
	void AttachMessageListener();
	void DetachMessageListener();

	void OnMessageReceivedFromBackground(Platform::Object^ sender, Windows::Media::Playback::MediaPlayerDataReceivedEventArgs^ e);
private:
	Windows::Foundation::EventRegistrationToken messageListenerToken;
};

END_NS_MEDIA