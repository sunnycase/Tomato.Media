//
// Tomato Media
// Media Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#include "pch.h"
#include "MediaSink.h"
#include "StreamSink.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

MediaSink::MediaSink()
{

}

HRESULT MediaSink::GetCharacteristics(DWORD * pdwCharacteristics)
{
	*pdwCharacteristics = 0;

	return S_OK;
}

HRESULT MediaSink::AddStreamSink(DWORD dwStreamSinkIdentifier, IMFMediaType * pMediaType, IMFStreamSink ** ppStreamSink)
{
	if (streamSinks.find(dwStreamSinkIdentifier) != streamSinks.end())
		return MF_E_STREAMSINK_EXISTS;

	try
	{
		auto streamSink = Make<StreamSink>(dwStreamSinkIdentifier, this);
		if (pMediaType)
			ThrowIfFailed(streamSink->SetCurrentMediaType(pMediaType));
		streamSinks.emplace(dwStreamSinkIdentifier, streamSink);
		*ppStreamSink = streamSink.Detach();
	}
	CATCH_ALL();

	return S_OK;
}

HRESULT MediaSink::RemoveStreamSink(DWORD dwStreamSinkIdentifier)
{
	auto it = streamSinks.find(dwStreamSinkIdentifier);
	if (it == streamSinks.end())
		return MF_E_INVALIDSTREAMNUMBER;
	streamSinks.erase(it);
	return S_OK;
}

HRESULT MediaSink::GetStreamSinkCount(DWORD * pcStreamSinkCount)
{
	*pcStreamSinkCount = streamSinks.size();
	return S_OK;
}

HRESULT MediaSink::GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink ** ppStreamSink)
{
	DWORD cntIndex = 0;
	for (auto&& pair : streamSinks)
	{
		if(cntIndex == dwIndex)
			return pair.second.CopyTo(ppStreamSink);
		cntIndex++;
	}
	return MF_E_INVALIDINDEX;
}

HRESULT MediaSink::GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink ** ppStreamSink)
{
	auto it = streamSinks.find(dwStreamSinkIdentifier);
	if (it == streamSinks.end())
		return MF_E_INVALIDSTREAMNUMBER;
	return it->second.CopyTo(ppStreamSink);
}

HRESULT MediaSink::SetPresentationClock(IMFPresentationClock * pPresentationClock)
{
	try
	{
		if (presentationClock.Get() != pPresentationClock)
		{
			if (presentationClock)
				ThrowIfFailed(presentationClock->RemoveClockStateSink(this));
			presentationClock = pPresentationClock;
			ThrowIfFailed(pPresentationClock->AddClockStateSink(this));
		}
	}
	CATCH_ALL();

	return S_OK;
}

HRESULT MediaSink::GetPresentationClock(IMFPresentationClock ** ppPresentationClock)
{
	if (!presentationClock) return MF_E_NO_CLOCK;
	return presentationClock.CopyTo(ppPresentationClock);
}

HRESULT MediaSink::Shutdown(void)
{
	return S_OK;
}

HRESULT MediaSink::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
	return E_NOTIMPL;
}

HRESULT MediaSink::OnClockStop(MFTIME hnsSystemTime)
{
	return E_NOTIMPL;
}

HRESULT MediaSink::OnClockPause(MFTIME hnsSystemTime)
{
	return E_NOTIMPL;
}

HRESULT MediaSink::OnClockRestart(MFTIME hnsSystemTime)
{
	return E_NOTIMPL;
}

HRESULT MediaSink::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
	return E_NOTIMPL;
}
