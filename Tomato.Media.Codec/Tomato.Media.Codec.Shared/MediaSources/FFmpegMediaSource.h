//
// Tomato Media Codec
// Media Foundation FFmpeg MediaSource
// 
// 作者：SunnyCase
// 创建时间：2015-12-30
#pragma once
#include "MediaSourceBase.h"
#include "MediaStreams/FFmpegDeliverMediaStream.h"
#include "../../include/MFWorkerQueueProvider.h"
#include "../../include/MFAsyncCallback.h"
#include "Utilities/FFmpegHelper.h"
#include <map>
#include <condition_variable>

DEFINE_NS_MEDIA_CODEC

// Media Foundation FFmpeg MediaSource
class FFmpegMediaSource : public MediaSourceBase
{
public:
	FFmpegMediaSource();
	virtual ~FFmpegMediaSource();

	// IMFGetService
	IFACEMETHOD(GetService) (_In_ REFGUID guidService, _In_ REFIID riid, _Out_opt_ LPVOID *ppvObject);

	// 通过 MediaSourceBase 继承
	virtual DWORD OnGetCharacteristics() override;
	virtual void OnValidatePresentationDescriptor(IMFPresentationDescriptor * pPD) override;
	virtual concurrency::task<WRL::ComPtr<IMFPresentationDescriptor>> OnCreatePresentationDescriptor(IMFByteStream * stream) override;
	virtual concurrency::task<void> OnStreamsRequestData(IMFMediaStream* mediaStream) override;
	virtual void OnSeekSource(MFTIME position) override;
	virtual void OnStartStream(DWORD streamId, bool selected, const PROPVARIANT& position) override;
	virtual void OnPauseStream(DWORD streamId) override;
	virtual void OnStopStream(DWORD streamId) override;
private:
	concurrency::task<WRL::ComPtr<IMFPresentationDescriptor>> BuildPresentationDescriptor(IMFByteStream * stream);

	///<remarks>调用前需对状态加锁</remarks>
	void RegisterWorkThreadIfNeeded();

	void EndOfSource();
	void AddStream(AVStream* stream, IMFStreamDescriptor* streamDescriptor);
private:
	Core::MFWorkerQueueProviderRef workerQueue;
	bool workThreadRegistered = false;
	std::shared_ptr<FFmpeg::Wrappers::AVFormatContextWrapper> _fmtContext;
	std::map<int, WRL::ComPtr<FFmpegDeliverMediaStream>> _streamIdMaps;
	std::map<int, WRL::ComPtr<FFmpegDeliverMediaStream>> _streamIndexMaps;
	WRL::ComPtr<IMFMetadataProvider> _metadataProvider;
};

END_NS_MEDIA_CODEC