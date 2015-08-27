//
// Tomato Media
// 呈现时钟
// 作者：SunnyCase
// 创建时间：2015-08-07
//
#include "pch.h"
#include "PresentationClock.h"
#include "Utility/MFPresentationTimeSource.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;
using namespace std::chrono;

PresentationClock::PresentationClock()
{

}

HRESULT PresentationClock::GetClockCharacteristics(DWORD * pdwCharacteristics)
{
	return clock ? clock->GetClockCharacteristics(pdwCharacteristics) : MF_E_NO_CLOCK;
}

HRESULT PresentationClock::GetCorrelatedTime(DWORD dwReserved, LONGLONG * pllClockTime, MFTIME * phnsSystemTime)
{
	return clock ? clock->GetCorrelatedTime(dwReserved, pllClockTime, phnsSystemTime) : MF_E_NO_CLOCK;
}

HRESULT PresentationClock::GetContinuityKey(DWORD * pdwContinuityKey)
{
	return clock ? clock->GetContinuityKey(pdwContinuityKey) : MF_E_NO_CLOCK;
}

HRESULT PresentationClock::GetState(DWORD dwReserved, MFCLOCK_STATE * peClockState)
{
	return clock ? clock->GetState(dwReserved, peClockState) : MF_E_NO_CLOCK;
}

HRESULT PresentationClock::GetProperties(MFCLOCK_PROPERTIES * pClockProperties)
{
	return clock ? clock->GetProperties(pClockProperties) : MF_E_NO_CLOCK;
}

HRESULT PresentationClock::SetTimeSource(IMFPresentationTimeSource * pTimeSource)
{
	try
	{
		if (pTimeSource)
		{
			ComPtr<IMFClockStateSink> clockStateSink; unsigned __int64 frequency; DWORD character;
			ThrowIfFailed(pTimeSource->QueryInterface(IID_PPV_ARGS(&clockStateSink)));
			ThrowIfFailed(pTimeSource->GetClockCharacteristics(&character));
			if (character & MFCLOCK_CHARACTERISTICS_FLAG_FREQUENCY_10MHZ)
				frequency = MFCLOCK_FREQUENCY_HNS;
			else
			{
				MFCLOCK_PROPERTIES prop;
				ThrowIfFailed(pTimeSource->GetProperties(&prop));
				frequency = prop.qwClockFrequency;
			}
			// 频率不能为 0
			if (!frequency) return E_INVALIDARG;

			this->clockStateSink.Swap(std::move(clockStateSink));
			this->frequency = frequency;
		}
		else
			clockStateSink.Reset();
		clock.Swap(pTimeSource);
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT PresentationClock::GetTimeSource(IMFPresentationTimeSource ** ppTimeSource)
{
	return clock ? clock.CopyTo(ppTimeSource) : MF_E_NO_CLOCK;
}

HRESULT PresentationClock::GetTime(MFTIME * phnsClockTime)
{
	ARGUMENT_NOTNULL_HR(phnsClockTime);

	try
	{
		if (clock)
		{
			LONGLONG time; MFTIME systemTime;
			ThrowIfFailed(clock->GetCorrelatedTime(0, &time, &systemTime));
			if (frequency == MFCLOCK_FREQUENCY_HNS)
				*phnsClockTime = time;
			else
				*phnsClockTime = time * MFCLOCK_FREQUENCY_HNS / frequency;
			return S_OK;
		}
	}
	CATCH_ALL();
	return MF_E_NO_CLOCK;
}

HRESULT PresentationClock::AddClockStateSink(IMFClockStateSink * pStateSink)
{
	ARGUMENT_NOTNULL_HR(pStateSink);

	if (std::find_if(clockStateSinks.begin(), clockStateSinks.end(), [=](auto sink) { return sink.Get() == pStateSink; }) == clockStateSinks.end())
	{
		clockStateSinks.emplace_back(pStateSink);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT PresentationClock::RemoveClockStateSink(IMFClockStateSink * pStateSink)
{
	ARGUMENT_NOTNULL_HR(pStateSink);

	auto it = std::find_if(clockStateSinks.begin(), clockStateSinks.end(), [=](auto sink) { return sink.Get() == pStateSink; });
	if (it != clockStateSinks.end())
	{
		clockStateSinks.erase(it);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT PresentationClock::Start(LONGLONG llClockStartOffset)
{
	if (!clock) return MF_E_NO_CLOCK;
	try
	{
		if (llClockStartOffset == PRESENTATION_CURRENT_POSITION)
		{
			MFCLOCK_STATE state;
			ThrowIfFailed(clock->GetState(0, &state));
			auto systemTime = duration_cast<hnseconds>(high_resolution_clock::now().time_since_epoch()).count();

			if (state == MFCLOCK_STATE_PAUSED)
			{
				ThrowIfFailed(clockStateSink->OnClockRestart(systemTime));
				parallel_for_each(clockStateSinks.begin(), clockStateSinks.end(), [=](auto sink) { sink->OnClockRestart(systemTime); });
			}
			else
			{
				ThrowIfFailed(clockStateSink->OnClockStart(systemTime, llClockStartOffset));
				parallel_for_each(clockStateSinks.begin(), clockStateSinks.end(), [=](auto sink) { sink->OnClockStart(systemTime, llClockStartOffset); });
			}
		}
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT PresentationClock::Stop(void)
{
	if (!clock) return MF_E_NO_CLOCK;
	try
	{
		auto systemTime = duration_cast<hnseconds>(high_resolution_clock::now().time_since_epoch()).count();
		ThrowIfFailed(clockStateSink->OnClockStop(systemTime));
		parallel_for_each(clockStateSinks.begin(), clockStateSinks.end(), [=](auto sink) { sink->OnClockStop(systemTime); });
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT PresentationClock::Pause(void)
{
	if (!clock) return MF_E_NO_CLOCK;
	try
	{
		auto systemTime = duration_cast<hnseconds>(high_resolution_clock::now().time_since_epoch()).count();
		ThrowIfFailed(clockStateSink->OnClockPause(systemTime));
		parallel_for_each(clockStateSinks.begin(), clockStateSinks.end(), [=](auto sink) { sink->OnClockPause(systemTime); });
	}
	CATCH_ALL();
	return S_OK;
}

ComPtr<IMFPresentationClock> NS_MEDIA::CreateHighResolutionPresentationClock()
{
	auto clock = Make<PresentationClock>();
	auto timeSource = Make<MFPresentationTimeSource>();
	ThrowIfFailed(clock->SetTimeSource(timeSource.Get()));

	return clock;
}
