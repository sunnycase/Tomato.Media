//
// Tomato Media
// Media Sink
// 
// 作者：SunnyCase 
// 创建日期 2015-08-07
#include "pch.h"
#include "MediaRenderSink.h"
#include "VideoStreamRenderSink.h"
#include "D3D11VideoRender.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

MediaRenderSink::MediaRenderSink()
	:audioSink(streamSinks[0]), videoSink(streamSinks[1])
{
}

HRESULT MediaRenderSink::GetCharacteristics(DWORD * pdwCharacteristics)
{
	*pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_CAN_PREROLL;

	return S_OK;
}

HRESULT MediaRenderSink::AddStreamSink(DWORD dwStreamSinkIdentifier, IMFMediaType * pMediaType, IMFStreamSink ** ppStreamSink)
{
	return MF_E_STREAMSINKS_FIXED;
}

HRESULT MediaRenderSink::RemoveStreamSink(DWORD dwStreamSinkIdentifier)
{
	return MF_E_STREAMSINKS_FIXED;
}

HRESULT MediaRenderSink::GetStreamSinkCount(DWORD * pcStreamSinkCount)
{
	*pcStreamSinkCount = streamSinks.size();
	return S_OK;
}

ComPtr<IVideoRender>& MediaRenderSink::GetVideoRender()
{
	if (!videoRender)
		videoRender = Make<D3D11VideoRender>();
	return videoRender;
}

HRESULT MediaRenderSink::NotifyPreroll(MFTIME hnsUpcomingStartTime)
{
	// 检查是否没有选中流
	if (std::all_of(streamSinks.begin(), streamSinks.end(), [](auto sink) {return !sink;}))
		return MF_E_SINK_NO_STREAMS;

	// 提醒每个 Stream Sink 开始缓冲
	try
	{
		std::for_each(streamSinks.begin(), streamSinks.end(), [=](auto sink) {
			sink->NotifyPreroll(hnsUpcomingStartTime); });
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT MediaRenderSink::GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink ** ppStreamSink)
{
	if (dwIndex >= streamSinks.size())
		return MF_E_INVALIDINDEX;
	try
	{
		std::lock_guard<decltype(stateMutex)> locker(stateMutex);
		if (dwIndex == 0 && !audioSink)
			audioSink = nullptr;//Make<VideoStreamRenderSink>(0, this, std::make_shared<D3D11VideoRender>());
		else if (dwIndex == 1 && !videoSink)
			videoSink = Make<VideoStreamRenderSink>(1, this, GetVideoRender().Get());

		return streamSinks[dwIndex].CopyTo(ppStreamSink);
	}
	CATCH_ALL();
}

HRESULT MediaRenderSink::GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink ** ppStreamSink)
{
	if (dwStreamSinkIdentifier >= streamSinks.size())
		return MF_E_INVALIDSTREAMNUMBER;
	return GetStreamSinkByIndex(dwStreamSinkIdentifier, ppStreamSink);
}

HRESULT MediaRenderSink::SetPresentationClock(IMFPresentationClock * pPresentationClock)
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

HRESULT MediaRenderSink::GetPresentationClock(IMFPresentationClock ** ppPresentationClock)
{
	if (!presentationClock) return MF_E_NO_CLOCK;
	return presentationClock.CopyTo(ppPresentationClock);
}

HRESULT MediaRenderSink::Shutdown(void)
{
	for (auto&& streamSink : streamSinks)
		streamSink.Reset();
	return S_OK;
}

HRESULT MediaRenderSink::OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset)
{
	return E_NOTIMPL;
}

HRESULT MediaRenderSink::OnClockStop(MFTIME hnsSystemTime)
{
	return E_NOTIMPL;
}

HRESULT MediaRenderSink::OnClockPause(MFTIME hnsSystemTime)
{
	return E_NOTIMPL;
}

HRESULT MediaRenderSink::OnClockRestart(MFTIME hnsSystemTime)
{
	return E_NOTIMPL;
}

HRESULT MediaRenderSink::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
{
	return E_NOTIMPL;
}

HRESULT MediaRenderSink::GetService(REFGUID guidService, REFIID riid, LPVOID * ppvObject)
{
	if (guidService == MF_TM_VIDEORENDER_SERVICE)
	{
		std::lock_guard<decltype(stateMutex)> locker(stateMutex);
		return GetVideoRender().CopyTo(riid, ppvObject);
	}
	return MF_E_UNSUPPORTED_SERVICE;
}
