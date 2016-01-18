//
// Tomato Media
// 支持 Effect 的 MediaStreamSource
// 作者：SunnyCase
// 创建时间：2016-01-13
//
#pragma once
#include "common.h"
#include "MediaSource.h"
#include <mutex>
#include <queue>
#include <atomic>
#include <ppltasks.h>

DEFINE_NS_MEDIA

public ref class EffectMediaStreamSource sealed
{
	struct SampleRequest
	{
		Windows::Media::Core::MediaStreamSourceSampleRequest^ Request;
		Platform::Agile<Windows::Media::Core::MediaStreamSourceSampleRequestDeferral^> Deferral;

		SampleRequest(Windows::Media::Core::MediaStreamSourceSampleRequest^ request, Windows::Media::Core::MediaStreamSourceSampleRequestDeferral^ deferral)
			:Request(request), Deferral(deferral)
		{

		}
	};
public:
	EffectMediaStreamSource(MediaSource^ mediaSource);

	property Windows::Media::Core::MediaStreamSource^ Source
	{
		Windows::Media::Core::MediaStreamSource^ get() { return _mss; }
	}

	void AddTransform(Windows::Media::IMediaExtension^ transform);
	void RemoveAllTransform();
private:
	void ConfigureSourceReader(IMFMediaSource* mediaSource);
	void InstallEffects();
	void ConfigureMSS();
	void DispatchSampleRequests();
	void ClearSampleAndRequestQueue();

	void OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample);
	void OnFlush(DWORD dwStreamIndex);

	void OnStarting(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^args);
	void OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);
	void OnClosed(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args);
private:
	Windows::Media::Core::MediaStreamSource^ _mss;
	WRL::ComPtr<IMFSourceReaderEx> _sourceReader;
	WRL::ComPtr<IMFMediaType> _outputMT;
	std::queue<WRL::ComPtr<IMFSample>> _sampleCache;
	std::queue<SampleRequest> _requestCache;
	std::queue<concurrency::task_completion_event<void>> _flushOperations;
	std::mutex _sampleCacheMutex, _requestCacheMutex, _flushOperationsMutex;
	std::atomic<bool> _endOfStream;
};

END_NS_MEDIA