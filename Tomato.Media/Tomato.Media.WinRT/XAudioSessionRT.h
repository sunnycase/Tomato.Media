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
public:
	XAudioSound(const Platform::Array<byte>^ wavData);

	property Windows::Media::MediaProperties::AudioEncodingProperties^ Format
	{
		Windows::Media::MediaProperties::AudioEncodingProperties^ get() { return _format; }
	}
internal:
	property const byte* Data
	{
		const byte* get() { return _data; }
	}

	property size_t DataSize
	{
		size_t get() { return _dataSize; }
	}
private:
	void InitializeSound();
private:
	const Platform::Array<byte>^ _wavData;
	Windows::Media::MediaProperties::AudioEncodingProperties^ _format;
	const byte* _data = nullptr;
	size_t _dataSize = 0;
};

public ref class XAudioChannel sealed
{
internal:
	XAudioChannel(std::shared_ptr<Internal::XAudioChannel>&& channel);

	property const std::shared_ptr<Internal::XAudioChannel>& Channel
	{
		const std::shared_ptr<Internal::XAudioChannel>& get() { return _channel; }
	}
public:
	property Windows::Media::MediaProperties::AudioEncodingProperties^ Format
	{
		Windows::Media::MediaProperties::AudioEncodingProperties^ get() { return _format; }
	}

	void Play(XAudioSound^ sound);
private:
	std::shared_ptr<Internal::XAudioChannel> _channel;
	Windows::Media::MediaProperties::AudioEncodingProperties^ _format;
};

///<summary>XAudio2 Audio Session</summary>
public ref class XAudioSession sealed
{
public:
	XAudioSession();

	XAudioChannel^ AddChannel(Windows::Media::MediaProperties::AudioEncodingProperties^ format);
private:
	Internal::XAudioSession _audioSession;
};

END_NS_MEDIA
