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

DEFINE_NS_MEDIA

class XAudioSound;

// XAudio2 Audio Session
class XAudioSession
{
public:
	XAudioSession();

	XAudioSound* AddSound(const WAVEFORMATEX* format, std::vector<byte>&& data);
	void PlaySound(XAudioSound* sound);
private:
	void InitializeDeviceResources();
private:
	WRL::ComPtr<IXAudio2> _xAudio;
	std::unique_ptr<IXAudio2MasteringVoice, Core::xaudio2_voice_deleter> _masteringVoice;
	std::vector<std::shared_ptr<XAudioSound>> _sounds;
};

END_NS_MEDIA