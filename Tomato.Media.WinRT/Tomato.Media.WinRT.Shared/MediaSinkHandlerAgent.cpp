//
// Tomato Media
// Media Sink 处理器代理
// 
// (c) SunnyCase 
// 创建日期 2015-05-13
#include "pch.h"
#include "AudioPlayer.h"
#include "MediaSinkHandlerAgent.h"

using namespace Tomato::Media;
using namespace Platform;

Tomato::Media::MediaSinkHandlerAgent::MediaSinkHandlerAgent(AudioPlayer ^ audioPlayer)
	:playerWeak(audioPlayer)
{

}

void MediaSinkHandlerAgent::OnStatusChanged(MediaSinkStatus status)
{
	GetPlayer()->OnSinkStatusChanged(status);
}

void MediaSinkHandlerAgent::OnMediaOpened()
{
	GetPlayer()->OnSinkMediaOpened();
}

void MediaSinkHandlerAgent::OnMediaEnded()
{
	GetPlayer()->OnSinkMediaEnded();
}

void MediaSinkHandlerAgent::OnSeekCompleted()
{
	GetPlayer()->OnSinkSeekCompleted();
}

void MediaSinkHandlerAgent::OnError(HRESULT error)
{
	GetPlayer()->OnSinkError(error);
}

AudioPlayer ^ MediaSinkHandlerAgent::GetPlayer() const
{
	AudioPlayer^ player = playerWeak.Resolve<AudioPlayer>();
	if (!player)
		throw ref new ObjectDisposedException();
	return player;
}
