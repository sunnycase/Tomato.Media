//
// Tomato Media
// 视频流渲染 Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#pragma once
#include "IVideoRender.h"
#include "StreamRenderSinkBase.h"
#include <atomic>

DEFINE_NS_MEDIA

///<summary>视频流 Sink</summary>
class VideoStreamRenderSink : public StreamRenderSinkBase
{
	// Sink 状态
	enum VideoStreamRenderSinkState
	{
		// 未加载（未设置媒体类型）
		NotInitialized,
		// 已加载（已设置媒体类型，且 Flush 后）
		Initialized,
		// 缓冲中
		Prerolling,
	};
public:
	VideoStreamRenderSink(DWORD identifier, MediaRenderSink* mediaSink, IVideoRender* videoRender);

	STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler ** ppHandler) override;
	STDMETHODIMP ProcessSample(IMFSample * pSample) override;
	STDMETHODIMP PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, const PROPVARIANT * pvarMarkerValue, const PROPVARIANT * pvarContextValue) override;
	STDMETHODIMP Flush(void) override;

	STDMETHODIMP IsMediaTypeSupported(IMFMediaType * pMediaType, IMFMediaType ** ppMediaType) override;
	STDMETHODIMP GetMediaTypeCount(DWORD * pdwTypeCount) override;
	STDMETHODIMP GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType ** ppType) override;
	STDMETHODIMP SetCurrentMediaType(IMFMediaType * pMediaType) override;
	STDMETHODIMP GetCurrentMediaType(IMFMediaType ** ppMediaType) override;
	STDMETHODIMP GetMajorType(GUID * pguidMajorType) override;

	virtual void NotifyPreroll(MFTIME hnsUpcomingStartTime) override;
private:
	void OnSetMediaType();
	///<param name="setInited">是否设置状态为 Initialized。</param>
	///<remarks>调用前需对状态加锁</remarks>
	void FlushCore(bool setInited = false);
	void PostSampleRequest();
	void PostSampleRequestIfNeeded();
private:
	UINT32 frameWidth, frameHeight;
	WRL::ComPtr<IVideoRender> videoRender;
	WRL::ComPtr<IMFMediaType> mediaType;
	std::mutex stateMutex;
	VideoStreamRenderSinkState sinkState = NotInitialized;
};

END_NS_MEDIA