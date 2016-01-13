#include "pch.h"
#include "EffectMediaStreamSource.h"
//
// Tomato Media
// 支持 Effect 的 MediaStreamSource
// 作者：SunnyCase
// 创建时间：2016-01-13
//
#include "pch.h"
#include "EffectMediaStreamSource.h"

using namespace Platform;
using namespace Windows::Media;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;
using namespace NS_MEDIA;
using namespace WRL;

EffectMediaStreamSource::EffectMediaStreamSource(MediaSource ^ mediaSource)
{
	ConfigureSourceReader(mediaSource->MFMediaSource);
	ConfigureMSS();
}

void EffectMediaStreamSource::ConfigureSourceReader(IMFMediaSource * mediaSource)
{
	ComPtr<IMFAttributes> attributes;
	ThrowIfFailed(MFCreateAttributes(&attributes, 3));
	// 设置 Callback
	ThrowIfFailed(attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this));
	ThrowIfFailed(attributes->SetString(MF_READWRITE_MMCSS_CLASS_AUDIO, L"Audio"));
	ThrowIfFailed(attributes->SetUINT32(MF_READWRITE_MMCSS_PRIORITY_AUDIO, 4));

	ComPtr<IMFSourceReader> sourceReader;
	ThrowIfFailed(MFCreateSourceReaderFromMediaSource(mediaSource, attributes.Get(), &sourceReader));
	ThrowIfFailed(sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false));
	ThrowIfFailed(sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true));
	ThrowIfFailed(sourceReader.As(&_sourceReader));

	ComPtr<IMFMediaType> mediaType;

	ThrowIfFailed(MFCreateMediaType(&mediaType));
	ThrowIfFailed(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
	ThrowIfFailed(mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float));
	ThrowIfFailed(sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mediaType.Get()));
	InstallEffects();
	ThrowIfFailed(sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &_outputMT));
}

void EffectMediaStreamSource::InstallEffects()
{
}

void EffectMediaStreamSource::ConfigureMSS()
{
	UINT32 sampleRate, channelCount, bitsPerSample;
	ThrowIfFailed(_outputMT->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate));
	ThrowIfFailed(_outputMT->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channelCount));
	ThrowIfFailed(_outputMT->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample));
	auto encProps = AudioEncodingProperties::CreatePcm(sampleRate, channelCount, bitsPerSample);
	encProps->Subtype = L"Float";
	auto streamDesc = ref new AudioStreamDescriptor(encProps);
	_mss = ref new MediaStreamSource(streamDesc);
}

HRESULT EffectMediaStreamSource::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
{
	return E_NOTIMPL;
}

HRESULT EffectMediaStreamSource::OnFlush(DWORD dwStreamIndex)
{
	return E_NOTIMPL;
}

HRESULT EffectMediaStreamSource::OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent)
{
	return E_NOTIMPL;
}
