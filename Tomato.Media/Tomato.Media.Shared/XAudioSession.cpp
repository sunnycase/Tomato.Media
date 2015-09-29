//
// Tomato Media
// XAudio2 Audio Session
// 
// 作者：SunnyCase 
// 创建日期 2015-09-26
#include "pch.h"
#include "XAudioSession.h"

using namespace NS_CORE;
using namespace NS_MEDIA_INTERN;
using namespace concurrency;
using namespace WRL;

class NS_MEDIA_INTERN::XAudioSound : public std::enable_shared_from_this<NS_MEDIA_INTERN::XAudioSound>
{
public:
	XAudioSound(const WAVEFORMATEX * format, IXAudio2* xAudio)
	{
		ThrowIfFailed(xAudio->CreateSourceVoice(&_sourceVoice._Myptr(), format));
		ThrowIfFailed(_sourceVoice->FlushSourceBuffers());
	}

	void Play()
	{
		XAUDIO2_BUFFER buffer = { 0 };
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		auto data = ProvideData();
		buffer.pAudioData = data.first;
		buffer.AudioBytes = data.second;
		ThrowIfFailed(_sourceVoice->SubmitSourceBuffer(&buffer));
		ThrowIfFailed(_sourceVoice->Start(0, XAUDIO2_COMMIT_NOW));
	}
protected:
	virtual std::pair<const byte*, size_t> ProvideData() = 0;
private:
	std::unique_ptr<IXAudio2SourceVoice, xaudio2_voice_deleter> _sourceVoice;
};

namespace
{
	class VectorDataXAudioSound : public XAudioSound
	{
	public:
		VectorDataXAudioSound(const WAVEFORMATEX * format, IXAudio2* xAudio, std::vector<byte>&& data)
			:XAudioSound(format, xAudio)
		{
			_data = std::move(data);
		}

	protected:
		virtual std::pair<const byte*, size_t> ProvideData() override
		{
			return{ _data.data(), _data.size() };
		}
	private:
		std::vector<byte> _data;
	};
}

XAudioSession::XAudioSession()
{
	InitializeDeviceResources();
}

NS_MEDIA_INTERN::XAudioSound* XAudioSession::AddSound(const WAVEFORMATEX * format, std::vector<byte>&& data)
{
	auto sound = std::make_shared<VectorDataXAudioSound>(format, _xAudio.Get(), std::move(data));
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
