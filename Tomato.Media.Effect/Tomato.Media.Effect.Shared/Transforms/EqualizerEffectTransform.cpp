//
// Tomato Media
// 均衡器 MFTransform
// 
// 作者：SunnyCase 
// 创建日期 2015-01-15
#include "pch.h"
#include "EqualizerEffectTransform.h"
#include "../../include/MFRAII.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_EFFECT;
using namespace concurrency;
using namespace WRL;

#if WINAPI_FAMILY==WINAPI_FAMILY_APP
ActivatableClass(EqualizerEffectTransform);
#else
CoCreatableClass(EqualizerEffectTransform);
#endif

EqualizerEffectTransform::EqualizerEffectTransform()
{

}

STDMETHODIMP EqualizerEffectTransform::AddOrUpdateFilter(FLOAT frequency, FLOAT bandWidth, FLOAT gain)
{
	std::lock_guard<decltype(_filtersMutex)> locker(_filtersMutex);
	_filterConfigs[frequency] = FilterConfig{ frequency, bandWidth, gain };
	_filtersConfigsDirty = true;
	return S_OK;
}

STDMETHODIMP EqualizerEffectTransform::RemoveFilter(FLOAT frequency)
{
	std::lock_guard<decltype(_filtersMutex)> locker(_filtersMutex);
	auto it = _filterConfigs.find(frequency);
	if (it != _filterConfigs.end())
		_filterConfigs.erase(frequency);
	_filtersConfigsDirty = true;
	return S_OK;
}

void EqualizerEffectTransform::OnValidateInputType(IMFMediaType * type)
{
	if (type)
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
}

void EqualizerEffectTransform::OnValidateOutputType(IMFMediaType * type)
{
	if (type)
	{
		GUID majorType, subType;

		// 验证主类型
		ThrowIfFailed(type->GetMajorType(&majorType));
		if (majorType != MFMediaType_Audio) ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
		// 验证子类型
		ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
		if (subType != MFAudioFormat_Float)
			ThrowIfFailed(MF_E_INVALIDMEDIATYPE);

		UINT32 bitsPerSample, blockAlign, sampleRate, channels;
		UINT32 bitsPerSample_i, blockAlign_i, sampleRate_i, channels_i;
		ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &blockAlign));
		ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample));
		ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate));
		ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels));

		ThrowIfFailed(inputMediaType->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &blockAlign_i));
		ThrowIfFailed(inputMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample_i));
		ThrowIfFailed(inputMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate_i));
		ThrowIfFailed(inputMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels_i));

		if (bitsPerSample != bitsPerSample_i || blockAlign != blockAlign_i ||
			sampleRate != sampleRate_i || channels != channels_i)
			ThrowIfFailed(MF_E_INVALIDMEDIATYPE);
	}
}

DWORD EqualizerEffectTransform::OnGetOutputFrameSize() const noexcept
{
	return _inputSize;
}

WRL::ComPtr<IMFMediaType> EqualizerEffectTransform::OnSetInputType(IMFMediaType * type)
{
	InitializeAvailableOutputTypes(type);
	return type;
}

WRL::ComPtr<IMFMediaType> EqualizerEffectTransform::OnSetOutputType(IMFMediaType * type)
{
	if (type)
	{
		GUID subType;
		UINT32 bitsPerSample;
		ThrowIfFailed(type->GetGUID(MF_MT_SUBTYPE, &subType));
		ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &_outputBlockAlign));
		ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample));
		ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &_outputSampleRate));
		ThrowIfFailed(type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &_outputChannels));

		InitializeEffectChain();
	}
	return type;
}

bool EqualizerEffectTransform::OnReceiveInput(IMFSample * sample)
{
	if (FAILED(sample->GetSampleTime(&_sampleTime)))
		_sampleTime = -1;
	if (FAILED(sample->GetSampleDuration(&_sampleDuration)))
		_sampleDuration = -1;
	ThrowIfFailed(sample->ConvertToContiguousBuffer(&_inputBuffer));
	ThrowIfFailed(_inputBuffer->GetCurrentLength(&_inputSize));
	return true;
}

void EqualizerEffectTransform::OnProduceOutput(MFT_OUTPUT_DATA_BUFFER & output)
{
	if (!output.pSample) ThrowIfFailed(E_INVALIDARG);
	ComPtr<IMFMediaBuffer> buffer;
	ThrowIfFailed(output.pSample->GetBufferByIndex(0, buffer.GetAddressOf()));
	BYTE *source, *dest; DWORD sourceSize, destSize;
	DWORD cntLength = 0;
	{
		Core::MFBufferLocker destLocker(buffer.Get());
		destLocker.Lock(dest, &destSize, nullptr);
		if (destSize < OnGetOutputFrameSize()) ThrowIfFailed(E_INVALIDARG);
		Core::MFBufferLocker sourceLocker(_inputBuffer.Get());
		sourceLocker.Lock(source, nullptr, &sourceSize);

		cntLength = FillFrame(source, sourceSize, dest, destSize);
	}
	ThrowIfFailed(buffer->SetCurrentLength(cntLength));
	if (_sampleTime != -1)
		ThrowIfFailed(output.pSample->SetSampleTime(_sampleTime));
	if (_sampleDuration != -1)
		ThrowIfFailed(output.pSample->SetSampleDuration(_sampleDuration));
}

namespace
{
	std::vector<ComPtr<IMFMediaType>> MakeInputTypes()
	{
		ComPtr<IMFMediaType> mediaType;

		ThrowIfFailed(MFCreateMediaType(&mediaType));
		ThrowIfFailed(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		ThrowIfFailed(mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float));
		return{ mediaType };
	}
}

WRL::ComPtr<IMFMediaType> EqualizerEffectTransform::OnGetInputAvailableType(DWORD index) noexcept
{
	static auto inputTypes = MakeInputTypes();
	if (index < inputTypes.size())
	{
		return inputTypes[index];
	}
	else
		return nullptr;
}

WRL::ComPtr<IMFMediaType> EqualizerEffectTransform::OnGetOutputAvailableType(DWORD index) noexcept
{
	if (index < availableOutputTypes.size())
	{
		return availableOutputTypes[index];
	}
	else
		return nullptr;
}

void EqualizerEffectTransform::InitializeAvailableOutputTypes(IMFMediaType * inputType)
{
	availableOutputTypes.clear();
	availableOutputTypes.emplace_back(inputType);
}

void EqualizerEffectTransform::InitializeEffectChain()
{
	MultiBandEqualizerFilter filter(_outputSampleRate);
	std::lock_guard<decltype(_filtersMutex)> locker(_filtersMutex);
	for (auto&& configPair : _filterConfigs)
	{
		const auto& config = configPair.second;
		try
		{
			filter.Add(config.frequency, config.bandWidth, config.gain);
		}
		catch (...) {}
	}

	_channelFilters.clear();
	for (size_t i = 0; i < _outputChannels; i++)
		_channelFilters.emplace_back(filter);
	_filtersConfigsDirty = false;
}

DWORD EqualizerEffectTransform::FillFrame(BYTE* source, DWORD sourceSize, BYTE* dest, DWORD destSize)
{
	const auto samples = sourceSize / _outputBlockAlign;
	const auto channels = size_t(_outputChannels);
	auto pSrc = reinterpret_cast<float*>(source);
	auto pDst = reinterpret_cast<float*>(dest);

	{
		std::lock_guard<decltype(_filtersMutex)> locker(_filtersMutex);
		if (_filtersConfigsDirty)
			InitializeEffectChain();
	}

	if (channels == 2)
	{
		for (size_t i = 0; i < samples; i++)
		{
			*pDst++ = _channelFilters[0].Process(*pSrc++);
			*pDst++ = _channelFilters[1].Process(*pSrc++);
		}
	}
	else if (channels == 1)
	{
		for (size_t i = 0; i < samples; i++)
			*pDst++ = _channelFilters[0].Process(*pSrc++);
	}
	else
	{
		for (size_t i = 0; i < samples; i++)
			for (size_t nCh = 0; nCh < channels; nCh++)
				*pDst++ = _channelFilters[nCh].Process(*pSrc++);
	}

	const auto totalSamples = samples * channels;
	pDst = reinterpret_cast<float*>(dest);
	for (size_t i = 0; i < totalSamples; i++, pDst++)
		*pDst = std::min(1.f, std::max(-1.f, *pDst));

	return samples * _outputBlockAlign;
}

void EqualizerEffectTransform::BeginStreaming()
{
}