//
// Tomato Media
// XAudio2 Audio Session
// 
// 作者：SunnyCase 
// 创建日期 2015-09-29
#pragma once
#include "XAudioSession.h"

DEFINE_NS_MEDIA

public ref class XAudioSound sealed
{
internal:
	XAudioSound(Internal::XAudioSound* sound)
		:_sound(sound)
	{

	}

	property Internal::XAudioSound* Sound
	{
		Internal::XAudioSound* get() { return _sound; }
	}
private:
	Internal::XAudioSound* _sound;
};

///<summary>XAudio2 Audio Session</summary>
public ref class XAudioSession sealed
{
public:
	XAudioSession();

	XAudioSound^ AddSound(const Platform::Array<byte>^ wavData);
	void PlaySound(XAudioSound^ sound);
private:
	Internal::XAudioSession _audioSession;
};

END_NS_MEDIA
