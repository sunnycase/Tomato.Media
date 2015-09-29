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

XAudioSession::XAudioSession()
{
}

XAudioSound^ XAudioSession::AddSound(const Platform::Array<byte>^ wavData)
{
	Riff::RiffReader reader;
	reader.ProvideData(wavData->Data, wavData->Length);

	Riff::RiffChunkHeader riffHeader;
	reader.FindRiffChunk(riffHeader);
	Riff::Chunk chunk;
	WAVEFORMATEX* waveFormat = nullptr; WAVEFORMAT* oldWaveFormat = nullptr;
	byte* data = nullptr; byte* dataEnd = nullptr;
	while (reader.FindChunk(chunk))
	{
		if (data && (waveFormat || oldWaveFormat))break;
		if (chunk.Header.Id == Riff::ChunkHeaders::Format)
		{
			if(chunk.Header.Size == 16)
				oldWaveFormat = reinterpret_cast<WAVEFORMAT*>(wavData->Data + chunk.Position);
			else if(chunk.Header.Size > 16)
				waveFormat = reinterpret_cast<WAVEFORMATEX*>(wavData->Data + chunk.Position);
		}
		else if (chunk.Header.Id == Riff::ChunkHeaders::Data)
		{
			data = wavData->Data + chunk.Position;
			dataEnd = data + chunk.Header.Size;
		}
	}
	ThrowIfNot(data && (waveFormat || oldWaveFormat), L"Invalid wave file.");
	if (waveFormat)
		return ref new XAudioSound(_audioSession.AddSound(waveFormat, { data, dataEnd }));
	else
	{
		WAVEFORMATEX format = { 0 };
		ThrowIfNot(memcpy_s(&format, sizeof(format), oldWaveFormat, sizeof(WAVEFORMAT)) == 0, L"Cannot copy memory.");
		format.wBitsPerSample = format.nBlockAlign * 8 / format.nChannels;
		return ref new XAudioSound(_audioSession.AddSound(&format, { data, dataEnd }));
	}
}

void XAudioSession::PlaySound(XAudioSound ^ sound)
{
	_audioSession.PlaySound(sound->Sound);
}
