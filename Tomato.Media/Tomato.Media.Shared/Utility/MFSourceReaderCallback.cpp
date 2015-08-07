//
// Tomato Media
// IMFSourceReaderCallback 实现
// 作者：SunnyCase
// 创建时间：2015-08-07
//
#include "pch.h"
#include "MFSourceReaderCallback.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

MFSourceReaderCallback::MFSourceReaderCallback(std::function<void(HRESULT, DWORD, DWORD, LONGLONG, IMFSample*)> sampleCallback)
	:sampleCallback(std::move(sampleCallback))
{
}

HRESULT MFSourceReaderCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
{
	try
	{
		if (sampleCallback)
			sampleCallback(hrStatus, dwStreamIndex, dwStreamFlags, llTimestamp, pSample);
	}
	CATCH_ALL();

	return S_OK;
}

HRESULT MFSourceReaderCallback::OnFlush(DWORD dwStreamIndex)
{
	return S_OK;
}

HRESULT MFSourceReaderCallback::OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent)
{
	return S_OK;
}
