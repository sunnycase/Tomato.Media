//
// Tomato Media
// Media Foundation PresentationTimeSource 实现
// 作者：SunnyCase
// 创建时间：2015-08-22
//
#include "pch.h"
#include "MFPresentationTimeSource.h"

using namespace NS_MEDIA;
using namespace WRL;

MFPresentationTimeSource::MFPresentationTimeSource()
{
}

HRESULT MFPresentationTimeSource::GetClockCharacteristics(DWORD * pdwCharacteristics)
{
	*pdwCharacteristics = 0;
	return S_OK;
}

HRESULT MFPresentationTimeSource::GetCorrelatedTime(DWORD dwReserved, LONGLONG * pllClockTime, MFTIME * phnsSystemTime)
{
	return E_NOTIMPL;
}

HRESULT MFPresentationTimeSource::GetContinuityKey(DWORD * pdwContinuityKey)
{
	return E_NOTIMPL;
}

HRESULT MFPresentationTimeSource::GetState(DWORD dwReserved, MFCLOCK_STATE * peClockState)
{
	return E_NOTIMPL;
}

HRESULT MFPresentationTimeSource::GetProperties(MFCLOCK_PROPERTIES * pClockProperties)
{
	return E_NOTIMPL;
}

HRESULT MFPresentationTimeSource::GetUnderlyingClock(IMFClock ** ppClock)
{
	return MF_E_NO_CLOCK;
}
