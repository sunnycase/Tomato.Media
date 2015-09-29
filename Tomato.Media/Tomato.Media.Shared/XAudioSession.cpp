//
// Tomato Media
// XAudio2 Audio Session
// 
// 作者：SunnyCase 
// 创建日期 2015-09-26
#include "pch.h"
#include "XAudioSession.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

class NS_MEDIA::XAudioSound : public std::enable_shared_from_this<NS_MEDIA::XAudioSound>
{
public:
	XAudioSound(const WAVEFORMATEX * format, std::vector<byte>&& data, IXAudio2* xAudio)
	{
		ThrowIfFailed(xAudio->CreateSourceVoice(&_sourceVoice._Myptr(), format));
		_data = std::move(data);
	}

	void Play()
	{
		XAUDIO2_BUFFER buffer = { 0 };
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		buffer.pAudioData = _data.data();
		buffer.AudioBytes = _data.size();
		ThrowIfFailed(_sourceVoice->SubmitSourceBuffer(&buffer));
		ThrowIfFailed(_sourceVoice->Start(0, XAUDIO2_COMMIT_NOW));
	}
private:
	std::unique_ptr<IXAudio2SourceVoice, xaudio2_voice_deleter> _sourceVoice;
	std::vector<byte> _data;
};

XAudioSession::XAudioSession()
{
}

NS_MEDIA::XAudioSound* XAudioSession::AddSound(const WAVEFORMATEX * format, std::vector<byte>&& data)
{
	auto sound = std::make_shared<XAudioSound>(format, std::move(data));
	auto ptr = sound.get();
	_sounds.emplace_back(std::move(sound));
	return ptr;
}

void XAudioSession::PlaySound(XAudioSound * sound)
{
	sound->Play();
}

void XAudioSession::InitializeDeviceResources()
{
	ThrowIfFailed(XAudio2Create(&_xAudio));
	ThrowIfFailed(_xAudio->CreateMasteringVoice(&_masteringVoice._Myptr()));
}
