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

MFSourceReaderCallback::MFSourceReaderCallback(std::function<void(HRESULT, DWORD, DWORD, LONGLONG, IMFSample*)>&& sampleCallback,
	std::function<void(DWORD)>&& flushCallback)
	:sampleCallback(std::move(sampleCallback)), flushCallback(std::move(flushCallback))
{
}

HRESULT MFSourceReaderCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
{
	try
	{
		sampleCallback(hrStatus, dwStreamIndex, dwStreamFlags, llTimestamp, pSample);
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT MFSourceReaderCallback::OnFlush(DWORD dwStreamIndex)
{
	try
	{
		flushCallback(dwStreamIndex);
	}
	CATCH_ALL();
	return S_OK;
}

HRESULT MFSourceReaderCallback::OnEvent(DWORD dwStreamIndex, IMFMediaEvent * pEvent)
{
	return S_OK;
}
