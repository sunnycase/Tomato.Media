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

DEFINE_NS_MEDIA

///<summary>Media Foundation PresentationTimeSource 实现</summary>
class MFPresentationTimeSource final : public WRL::RuntimeClass<WRL::RuntimeClassFlags<
	WRL::ClassicCom>, IMFPresentationTimeSource>
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
private:
};

END_NS_MEDIA