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

XAudioChannel::XAudioChannel(const WAVEFORMATEX * format, IXAudio2* xAudio)
	:_format(*format)
{
	ThrowIfFailed(xAudio->CreateSourceVoice(&_sourceVoice._Myptr(), format));
	ThrowIfFailed(_sourceVoice->FlushSourceBuffers());
}

void XAudioChannel::Play(const byte* data, size_t size)
{
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.pAudioData = data;
	buffer.AudioBytes = size;
	ThrowIfFailed(_sourceVoice->SubmitSourceBuffer(&buffer));
	ThrowIfFailed(_sourceVoice->Start(0, XAUDIO2_COMMIT_NOW));
}

XAudioSession::XAudioSession()
{
	InitializeDeviceResources();
}

std::shared_ptr<XAudioChannel> XAudioSession::AddChannel(const WAVEFORMATEX * format)
{
	return std::make_shared<XAudioChannel>(format, _xAudio.Get());
}

void XAudioSession::InitializeDeviceResources()
{
	ThrowIfFailed(XAudio2Create(&_xAudio));
	ThrowIfFailed(_xAudio->CreateMasteringVoice(&_masteringVoice._Myptr()));
}
