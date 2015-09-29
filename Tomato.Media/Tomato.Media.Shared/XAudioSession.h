//
// Tomato Media
// XAudio2 Audio Session
// 
// 作者：SunnyCase 
// 创建日期 2015-09-26
#pragma once
#include "common.h"
#include <xaudio2.h>
#include "../../include/XAudio2RAII.h"

DEFINE_NS_MEDIA_INTERN

class XAudioChannel : public std::enable_shared_from_this<XAudioChannel>
{
public:
	XAudioChannel(const WAVEFORMATEX* format, IXAudio2* xAudio);

	// 播放完毕之前保证 data 有效
	void Play(const byte* data, size_t size);
	const WAVEFORMATEX& GetFormat() const noexcept { return _format; }
private:
	std::unique_ptr<IXAudio2SourceVoice, Core::xaudio2_voice_deleter> _sourceVoice;
	WAVEFORMATEX _format;
};

// XAudio2 Audio Session
class XAudioSession
{
public:
	XAudioSession();

	std::shared_ptr<XAudioChannel> AddChannel(const WAVEFORMATEX* format);
private:
	void InitializeDeviceResources();
private:
	WRL::ComPtr<IXAudio2> _xAudio;
	std::unique_ptr<IXAudio2MasteringVoice, Core::xaudio2_voice_deleter> _masteringVoice;
};

END_NS_MEDIA_INTERN