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
using namespace std::chrono;

#define SPIN_COUNT 4000

MFPresentationTimeSource::MFPresentationTimeSource()
	:stateLock(SPIN_COUNT)
{
}

HRESULT MFPresentationTimeSource::GetClockCharacteristics(DWORD * pdwCharacteristics)
{
	ARGUMENT_NOTNULL_HR(pdwCharacteristics);

	// 时间单位为 100ns
	*pdwCharacteristics = MFCLOCK_CHARACTERISTICS_FLAG_FREQUENCY_10MHZ;
	return S_OK;
}

HRESULT MFPresentationTimeSource::GetCorrelatedTime(DWORD dwReserved, LONGLONG * pllClockTime, MFTIME * phnsSystemTime)
{
	ARGUMENT_NOTNULL_HR(pllClockTime || phnsSystemTime);

	{
		auto locker(stateLock.Lock());

		auto cntTimePoint = high_resolution_clock::now();
		*phnsSystemTime = duration_cast<hnseconds>(cntTimePoint.time_since_epoch()).count();

		auto cntState = state;
		switch (cntState)
		{
		case MFCLOCK_STATE_PAUSED:
			*pllClockTime = presentationFix.count();
			break;
		case MFCLOCK_STATE_RUNNING:
			*pllClockTime = duration_cast<hnseconds>(cntTimePoint - beginTimePoint + presentationFix).count();
			break;
		case MFCLOCK_STATE_STOPPED:
			*pllClockTime = 0;
		default:
			return E_NOT_VALID_STATE;
		}
	}
	*pllClockTime = static_cast<MFTIME>(*pllClockTime * rate.load(std::memory_order_consume));
	return S_OK;
}

HRESULT MFPresentationTimeSource::GetContinuityKey(DWORD * pdwContinuityKey)
{
	ARGUMENT_NOTNULL_HR(pdwContinuityKey);

	*pdwContinuityKey = 0;
	return S_OK;
}

HRESULT MFPresentationTimeSource::GetState(DWORD dwReserved, MFCLOCK_STATE * peClockState)
{
	ARGUMENT_NOTNULL_HR(peClockState);

	auto locker(stateLock.Lock());
	*peClockState = state;
	return S_OK;
}

HRESULT MFPresentationTimeSource::GetProperties(MFCLOCK_PROPERTIES * pClockProperties)
{
	ARGUMENT_NOTNULL_HR(pClockProperties);

	*pClockProperties =
	{
		0, GUID_NULL, 0, MFCLOCK_FREQUENCY_HNS, MFCLOCK_TOLERANCE_UNKNOWN, MFCLOCK_JITTER_ISR
	};
	return S_OK;
}

HRESULT MFPresentationTimeSource::GetUnderlyingClock(IMFClock ** ppClock)
{
	return MF_E_NO_CLOCK;
}

HRESULT MFPresentationTimeSource::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
	auto locker(stateLock.Lock());

	auto cntState = state;
	switch (cntState)
	{
	case MFCLOCK_STATE_RUNNING:
		if (llClockStartOffset != PRESENTATION_CURRENT_POSITION)
		{
			presentationFix = hnseconds(llClockStartOffset);
			beginTimePoint = high_resolution_clock::now();
		}
		break;
	case MFCLOCK_STATE_PAUSED:
	case MFCLOCK_STATE_STOPPED:
		if (llClockStartOffset != PRESENTATION_CURRENT_POSITION)
			presentationFix = hnseconds(llClockStartOffset);
		beginTimePoint = high_resolution_clock::now();
		state = MFCLOCK_STATE_RUNNING;
		break;
	default:
		return E_NOT_VALID_STATE;
	}

	return S_OK;
}

HRESULT MFPresentationTimeSource::OnClockStop(MFTIME hnsSystemTime)
{
	auto locker(stateLock.Lock());

	auto cntState = state;
	switch (cntState)
	{
	case MFCLOCK_STATE_RUNNING:
	case MFCLOCK_STATE_PAUSED:
		presentationFix = hnseconds::zero();
		state = MFCLOCK_STATE_STOPPED;
		break;
	case MFCLOCK_STATE_STOPPED:
		return S_FALSE;
	default:
		return E_NOT_VALID_STATE;
	}

	return S_OK;
}

HRESULT MFPresentationTimeSource::OnClockPause(MFTIME hnsSystemTime)
{
	auto locker(stateLock.Lock());

	auto cntState = state;
	switch (cntState)
	{
	case MFCLOCK_STATE_RUNNING:
		presentationFix += duration_cast<hnseconds>(high_resolution_clock::now() - beginTimePoint);
		state = MFCLOCK_STATE_PAUSED;
		break;
	case MFCLOCK_STATE_PAUSED:
		return S_FALSE;
	case MFCLOCK_STATE_STOPPED:
	default:
		return E_NOT_VALID_STATE;
	}

	return S_OK;
}

HRESULT MFPresentationTimeSource::OnClockRestart(MFTIME hnsSystemTime)
{
	auto locker(stateLock.Lock());

	if (state == MFCLOCK_STATE_PAUSED)
	{
		beginTimePoint = high_resolution_clock::now();
		state = MFCLOCK_STATE_RUNNING;
		return S_OK;
	}
	return E_NOT_VALID_STATE;
}

HRESULT MFPresentationTimeSource::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
	rate.store(flRate, std::memory_order_release);
	return S_OK;
}
