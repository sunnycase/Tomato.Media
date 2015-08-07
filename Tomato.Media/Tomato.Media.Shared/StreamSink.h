//
// Tomato Media
// Stream Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#pragma once
#include "common.h"
#include <mfidl.h>

DEFINE_NS_MEDIA

class StreamSink : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IMFStreamSink, IMFMediaTypeHandler>
{
public:
	StreamSink(DWORD identifier, IMFMediaSink* mediaSink);

	// 通过 RuntimeClass 继承
	STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent ** ppEvent) override;
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback * pCallback, IUnknown * punkState) override;
	STDMETHODIMP EndGetEvent(IMFAsyncResult * pResult, IMFMediaEvent ** ppEvent) override;
	STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT * pvValue) override;
	STDMETHODIMP GetMediaSink(IMFMediaSink ** ppMediaSink) override;
	STDMETHODIMP GetIdentifier(DWORD * pdwIdentifier) override;
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
private:
	WRL::ComPtr<IMFMediaEventQueue> eventQueue;
	IMFMediaSink* mediaSink;
	DWORD identifier;
	WRL::ComPtr<IMFMediaType> mediaType;
};

END_NS_MEDIA