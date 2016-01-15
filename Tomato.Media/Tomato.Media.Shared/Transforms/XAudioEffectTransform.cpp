//
// Tomato Media
// XAudio 音效 for MFTransform
// 
// 作者：SunnyCase 
// 创建日期 2015-01-15
#include "pch.h"
#include "XAudioEffectTransform.h"
#include "../../include/MFRAII.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

#if WINAPI_FAMILY==WINAPI_FAMILY_APP
ActivatableClass(XAudioEffectTransform);
#else
CoCreatableClass(XAudioEffectTransform);
#endif

XAudioEffectTransform::XAudioEffectTransform()
{
	InitializeXAudio();
}

void XAudioEffectTransform::OnValidateInputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Audio) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (subType != MFAudioFormat_Float)
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

void XAudioEffectTransform::OnValidateOutputType(IMFMediaType * type)
{
	GUID majorType, subType;

	// 验证主类型
	ThrowIfFailed(type->GetMajorType(&majorType));
	if (majorType != MFMediaType_Audio) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	// 验证子类型
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	if (subType != MFAudioFormat_Float)
		ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
}

DWORD XAudioEffectTransform::OnGetOutputFrameSize() const noexcept
{
	return 0;
}

WRL::ComPtr<IMFMediaType> XAudioEffectTransform::OnSetInputType(IMFMediaType * type)
{
	InitializeSourceVoice(type);
	InitializeAvailableOutputTypes();
	return type;
}

WRL::ComPtr<IMFMediaType> XAudioEffectTransform::OnSetOutputType(IMFMediaType * type)
{
	GUID subType;
	UINT32 bitsPerSample;
	ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &_outputBlockAlign));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &_outputSampleRate));
	ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &_outputChannels));
	bytesPerDecodecSample = _outputBlockAlign;

	InitializeEffectChain(type);
	return type;
}

bool XAudioEffectTransform::OnReceiveInput(IMFSample * sample)
{
	if (FAILED(sample->GetSampleTime(&_sampleTime)))
		_sampleTime = -1;
	ThrowIfFailed(sample->ConvertToContiguousBuffer(&_inputBuffer));
	BYTE* data; DWORD length;
	ThrowIfFailed(_inputBuffer->Lock(&data, nullptr, &length));
	return DecodeOneFrame();
}

void XAudioEffectTransform::OnProduceOutput(MFT_OUTPUT_DATA_BUFFER & output)
{
	if (!output.pSample) ThrowIfFailed(E_INVALIDARG);
	ComPtr<IMFMediaBuffer> buffer;
	ThrowIfFailed(output.pSample->GetBufferByIndex(0, buffer.GetAddressOf()));
	BYTE* data; DWORD maxLength;
	DWORD cntLength = 0;
	{
		Core::MFBufferLocker locker(buffer.Get());
		locker.Lock(data, &maxLength, nullptr);
		if (maxLength < OnGetOutputFrameSize()) ThrowIfFailed(E_INVALIDARG);

		cntLength = FillFrame(buffer.Get(), data, maxLength);
	}
	ThrowIfFailed(buffer->SetCurrentLength(cntLength));
	auto duration = MFTIME(decodedSamples * 1e7 / _outputSampleRate);
	ThrowIfFailed(output.pSample->SetSampleDuration(duration));
	if (_sampleTime != -1)
	{
		ThrowIfFailed(output.pSample->SetSampleTime(_sampleTime));
		_sampleTime += duration;
	}
	if (DecodeOneFrame())
		output.dwStatus = MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
}

bool XAudioEffectTransform::DecodeOneFrame()
{
	decodedSamples = 0;

	auto ret = [&] {

		return true;
	}();
	if (!ret && _inputBuffer)
	{
		_inputBuffer->Unlock();
		_inputBuffer.Reset();
	}
	return ret;
}

WRL::ComPtr<IMFMediaType> XAudioEffectTransform::OnGetOutputAvailableType(DWORD index) noexcept
{
	if (index < availableOutputTypes.size())
	{
		return availableOutputTypes[index];
	}
	else
		return nullptr;
}

void XAudioEffectTransform::InitializeXAudio()
{
	ThrowIfFailed(XAudio2Create(&_xAudio));
}

void XAudioEffectTransform::InitializeAvailableOutputTypes()
{
	availableOutputTypes.clear();

	availableOutputTypes.emplace_back(inputMediaType);
}

void XAudioEffectTransform::InitializeSourceVoice(IMFMediaType * inputType)
{
	unique_cotaskmem<WAVEFORMATEX> format;
	ThrowIfFailed(MFCreateWaveFormatExFromMFMediaType(inputType, &format._Myptr(), nullptr));
	IXAudio2SourceVoice* sourceVoice;
	ThrowIfFailed(_xAudio->CreateSourceVoice(&sourceVoice, format.get()));
	_sourceVoice.reset(sourceVoice);
}

void XAudioEffectTransform::InitializeEffectChain(IMFMediaType* outputType)
{
	IXAudio2SubmixVoice* submixVoice;
	ThrowIfFailed(_xAudio->CreateSubmixVoice(&submixVoice, _outputChannels, _outputSampleRate));
	_submixVoice.reset(submixVoice);
}

DWORD XAudioEffectTransform::FillFrame(IMFMediaBuffer* buffer, BYTE * data, DWORD maxLength)
{
	int converted = 0;
	return converted * _outputBlockAlign;
}

void XAudioEffectTransform::BeginStreaming()
{
}