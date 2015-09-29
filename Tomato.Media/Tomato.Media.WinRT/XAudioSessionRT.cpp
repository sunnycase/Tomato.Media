//
// Tomato Media
// XAudio2 Audio Session
// 
// 作者：SunnyCase 
// 创建日期 2015-09-29
#include "pch.h"
#include "XAudioSessionRT.h"
#include "../../include/riff.h"

using namespace NS_CORE;
using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;
using namespace Platform;
using namespace Windows::Media::MediaProperties;

XAudioChannel::XAudioChannel(std::shared_ptr<Internal::XAudioChannel>&& channel)
	:_channel(std::move(channel))
{
	auto wavFormat = _channel->GetFormat();
	_format = AudioEncodingProperties::CreatePcm(wavFormat.nSamplesPerSec, wavFormat.nChannels, wavFormat.wBitsPerSample);
}

void XAudioChannel::Play(XAudioSound ^ sound)
{
	_channel->Play(sound->Data, sound->DataSize);
}

XAudioSession::XAudioSession()
{
}

XAudioChannel^ XAudioSession::AddChannel(AudioEncodingProperties^ format)
{
	WAVEFORMATEX waveformat = { 0 };
	waveformat.nChannels = format->ChannelCount;
	waveformat.nSamplesPerSec = format->SampleRate;
	waveformat.wBitsPerSample = format->BitsPerSample;
	waveformat.wFormatTag = WAVE_FORMAT_PCM;
	waveformat.nBlockAlign = waveformat.wBitsPerSample * waveformat.nChannels / 8;
	waveformat.nAvgBytesPerSec = format->Bitrate / 8;
	return ref new XAudioChannel(_audioSession.AddChannel(&waveformat));
}

XAudioSound::XAudioSound(const Platform::Array<byte>^ wavData)
	:_wavData(wavData)
{
	InitializeSound();
}

void XAudioSound::InitializeSound()
{
	Riff::RiffReader reader;
	reader.ProvideData(_wavData->Data, _wavData->Length);

	Riff::RiffChunkHeader riffHeader;
	reader.FindRiffChunk(riffHeader);
	Riff::Chunk chunk;
	WAVEFORMATEX* waveFormat = nullptr; WAVEFORMAT* oldWaveFormat = nullptr;
	while (reader.FindChunk(chunk))
	{
		if (_data && (waveFormat || oldWaveFormat))break;
		if (chunk.Header.Id == Riff::ChunkHeaders::Format)
		{
			if (chunk.Header.Size == 16)
				oldWaveFormat = reinterpret_cast<WAVEFORMAT*>(_wavData->Data + chunk.Position);
			else if (chunk.Header.Size > 16)
				waveFormat = reinterpret_cast<WAVEFORMATEX*>(_wavData->Data + chunk.Position);
		}
		else if (chunk.Header.Id == Riff::ChunkHeaders::Data)
		{
			_data = _wavData->Data + chunk.Position;
			_dataSize = chunk.Header.Size;
		}
	}
	ThrowIfNot(_data && (waveFormat || oldWaveFormat), L"Invalid wave file.");
	if (waveFormat)
		_format = AudioEncodingProperties::CreatePcm(waveFormat->nSamplesPerSec, waveFormat->nChannels, waveFormat->wBitsPerSample);
	else
	{
		WAVEFORMATEX format = { 0 };
		ThrowIfNot(memcpy_s(&format, sizeof(format), oldWaveFormat, sizeof(WAVEFORMAT)) == 0, L"Cannot copy memory.");
		format.wBitsPerSample = format.nBlockAlign * 8 / format.nChannels;
		_format = AudioEncodingProperties::CreatePcm(format.nSamplesPerSec, format.nChannels, format.wBitsPerSample);
	}
}
