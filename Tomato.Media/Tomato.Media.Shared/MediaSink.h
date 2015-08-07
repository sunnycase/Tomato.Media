//
// Tomato Media
// Media Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#pragma once
#include "common.h"
#include <mfidl.h>
#include <map>

DEFINE_NS_MEDIA

class MediaSink : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IMFMediaSink, IMFClockStateSink>
{
public:
	MediaSink();

	// 通过 RuntimeClass 继承
	STDMETHODIMP GetCharacteristics(DWORD * pdwCharacteristics) override;
	STDMETHODIMP AddStreamSink(DWORD dwStreamSinkIdentifier, IMFMediaType * pMediaType, IMFStreamSink ** ppStreamSink) override;
	STDMETHODIMP RemoveStreamSink(DWORD dwStreamSinkIdentifier) override;
	STDMETHODIMP GetStreamSinkCount(DWORD * pcStreamSinkCount) override;
	STDMETHODIMP GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink ** ppStreamSink) override;
	STDMETHODIMP GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink ** ppStreamSink) override;
	STDMETHODIMP SetPresentationClock(IMFPresentationClock * pPresentationClock) override;
	STDMETHODIMP GetPresentationClock(IMFPresentationClock ** ppPresentationClock) override;
	STDMETHODIMP Shutdown(void) override;
	STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset) override;
	STDMETHODIMP OnClockStop(MFTIME hnsSystemTime) override;
	STDMETHODIMP OnClockPause(MFTIME hnsSystemTime) override;
	STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime) override;
	STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate) override;
private:
	std::map<DWORD, WRL::ComPtr<IMFStreamSink>> streamSinks;
	WRL::ComPtr<IMFPresentationClock> presentationClock;
};

END_NS_MEDIA