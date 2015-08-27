//
// Tomato Media
// Media Foundation PresentationTimeSource 实现
// 作者：SunnyCase
// 创建时间：2015-08-22
//
#pragma once
#include "common.h"
#include <mfidl.h>
#include <chrono>
#include <atomic>
#include <mutex>

DEFINE_NS_MEDIA

///<summary>Media Foundation PresentationTimeSource 实现</summary>
///<remarks>所有方法为线程安全。</remarks>
class MFPresentationTimeSource final : public WRL::RuntimeClass<WRL::RuntimeClassFlags<
	WRL::ClassicCom>, IMFPresentationTimeSource, IMFClockStateSink>
{
public:
	MFPresentationTimeSource();

	// 通过 RuntimeClass 继承
	STDMETHODIMP GetClockCharacteristics(DWORD * pdwCharacteristics) override;
	STDMETHODIMP GetCorrelatedTime(DWORD dwReserved, LONGLONG * pllClockTime, MFTIME * phnsSystemTime) override;
	STDMETHODIMP GetContinuityKey(DWORD * pdwContinuityKey) override;
	STDMETHODIMP GetState(DWORD dwReserved, MFCLOCK_STATE * peClockState) override;
	STDMETHODIMP GetProperties(MFCLOCK_PROPERTIES * pClockProperties) override;
	STDMETHODIMP GetUnderlyingClock(IMFClock ** ppClock) override;

	STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset) override;
	STDMETHODIMP OnClockStop(MFTIME hnsSystemTime) override;
	STDMETHODIMP OnClockPause(MFTIME hnsSystemTime) override;
	STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime) override;
	STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate) override;
private:
	hnseconds presentationFix = hnseconds::zero();				// 需要加上的时间量
	std::chrono::steady_clock::time_point beginTimePoint;		// 开始的时刻
	std::atomic<float> rate = 1.f;
	MFCLOCK_STATE state = MFCLOCK_STATE_STOPPED;
	WRL::Wrappers::CriticalSection stateLock;
};

END_NS_MEDIA