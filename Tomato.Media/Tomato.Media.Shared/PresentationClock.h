//
// Tomato Media
// 呈现时钟
// 作者：SunnyCase
// 创建时间：2015-08-07
//
#pragma once
#include "common.h"
#include <mfidl.h>

DEFINE_NS_MEDIA

///<summary>呈现时钟</summary>
///<remarks>实现 IUnknown 以外的所有方法不保证线程安全。</remarks>
class PresentationClock : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IMFPresentationClock>
{
public:
	PresentationClock();

	// 通过 RuntimeClass 继承
	STDMETHODIMP GetClockCharacteristics(DWORD * pdwCharacteristics) override;
	STDMETHODIMP GetCorrelatedTime(DWORD dwReserved, LONGLONG * pllClockTime, MFTIME * phnsSystemTime) override;
	STDMETHODIMP GetContinuityKey(DWORD * pdwContinuityKey) override;
	STDMETHODIMP GetState(DWORD dwReserved, MFCLOCK_STATE * peClockState) override;
	STDMETHODIMP GetProperties(MFCLOCK_PROPERTIES * pClockProperties) override;
	STDMETHODIMP SetTimeSource(IMFPresentationTimeSource * pTimeSource) override;
	STDMETHODIMP GetTimeSource(IMFPresentationTimeSource ** ppTimeSource) override;
	STDMETHODIMP GetTime(MFTIME * phnsClockTime) override;
	STDMETHODIMP AddClockStateSink(IMFClockStateSink * pStateSink) override;
	STDMETHODIMP RemoveClockStateSink(IMFClockStateSink * pStateSink) override;
	STDMETHODIMP Start(LONGLONG llClockStartOffset) override;
	STDMETHODIMP Stop(void) override;
	STDMETHODIMP Pause(void) override;
private:
	WRL::ComPtr<IMFClock> clock;
	WRL::ComPtr<IMFClockStateSink> clockStateSink;
	std::vector<WRL::ComPtr<IMFClockStateSink>> clockStateSinks;
	unsigned __int64 frequency = 0;
};

///<summary>创建以 high_resolution_clock 为内部 clock 的呈现时钟</summary>
WRL::ComPtr<IMFPresentationClock> CreateHighResolutionPresentationClock();

END_NS_MEDIA