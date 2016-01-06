//
// Tomato Media
// Windows::Meida::Core::IMediaSource 实现
// 作者：SunnyCase
// 创建时间：2015-08-05
//
#include "pch.h"
#include "CoreMediaSource.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace NS_MEDIA;

CoreMediaSource::CoreMediaSource(MediaSource^ mediaSource)
	:mediaSource(mediaSource)
{

}

CoreMediaSource::~CoreMediaSource()
{
}

HRESULT CoreMediaSource::GetService(REFGUID guidService, REFIID riid, LPVOID * ppvObject)
{
	if (guidService == MF_MEDIASOURCE_SERVICE)
	{
		if (riid == IID_IMFMediaSource)
		{
			auto mfSource = mediaSource->MFMediaSource;
			if (mfSource)
				return mfSource->QueryInterface(riid, ppvObject);
		}
	}
	return MF_E_UNSUPPORTED_SERVICE;
}
