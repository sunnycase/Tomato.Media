//
// Tomato Media
// [Internal] Media Foundation 媒体源读取器
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../MediaSources/IMediaSourceIntern.h"
#include "../include/ISourceReader.h"
#include "Utilities/MFMMCSSProvider.h"
#include "Utilities/block_buffer.hpp"

NSDEF_TOMATO_MEDIA

struct ReadSampleResult
{
	wrl::ComPtr<IMFSample> sample;
	DWORD streamFlags;
};

class MFSourceReaderCallback : public wrl::RuntimeClass<wrl::RuntimeClassFlags<
	wrl::RuntimeClassType::ClassicCom>, IMFSourceReaderCallback>
{
public:
	MFSourceReaderCallback();

	// 通过 RuntimeClass 继承
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample) noexcept;
	STDMETHODIMP OnFlush(DWORD dwStreamIndex) noexcept;
	STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent) noexcept;

	void SetReadSampleCallback(std::function<void(ReadSampleResult)>&& readSampleCallback);
	// 读取一个采样
	void BeginReadSample(IMFSourceReaderEx* sourceReader);
	void Stop();
private:
	std::function<void(ReadSampleResult)> readSampleCallback;
	std::recursive_mutex readMutex;
};

// Media Foundation 媒体源读取器
class MFSourceReader : public ISourceReader
{
public:
	const size_t PREROLL_DURATION_SEC = 3;

	MFSourceReader(IMediaSourceIntern* mediaSource);

	virtual void Start(int64_t hns);
	virtual void Stop();
	virtual size_t Read(byte* buffer, size_t bufferSize);
	virtual void SetAudioFormat(const WAVEFORMATEX* format, uint32_t framesPerPeriod);
	virtual SourceReaderState GetState() const { return readerState; }
	virtual void SetCurrentPosition(int64_t hns);
private:
	void InitializeOutputMediaType(const WAVEFORMATEX* outputFormat);
	void Initialize(wrl::ComPtr<IMFByteStream>&& byteStream);
	void ConfigureSourceReader();
	// 读取到采样后触发
	void OnSampleRead(ReadSampleResult result);
	void EnqueueSample(wrl::ComPtr<IMFSample>& sample);
	bool IsPreRollFilled();
	void NotifyReadSample();
private:
	wrl::ComPtr<MFSourceReaderCallback> sourceReaderCallback;
	wrl::ComPtr<IMFSourceReaderEx> sourceReader;
	wrl::ComPtr<IMFMediaType> outputMT;

	volatile SourceReaderState readerState = SourceReaderState::NotInitialized;
	MFMMCSSProvider mmcssProvider;
	block_buffer<byte> decodedBuffer;
	unique_cotaskmem<WAVEFORMATEX> outputFormat;
	size_t bytesPerPeriodLength;
	std::recursive_mutex stateMutex;
	concurrency::event requestEvent;
};

NSED_TOMATO_MEDIA