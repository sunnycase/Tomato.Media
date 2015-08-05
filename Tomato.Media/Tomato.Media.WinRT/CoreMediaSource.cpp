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

CoreMediaSource::CoreMediaSource(IMFMediaSource* mediaSource)
	:mediaSource(mediaSource)
{

}

HRESULT CoreMediaSource::GetService(REFGUID guidService, REFIID riid, LPVOID * ppvObject)
{
	if (guidService == MF_MEDIASOURCE_SERVICE)
	{
		if (riid == IID_IMFMediaSource)
		{
			mediaSource.CopyTo(reinterpret_cast<IMFMediaSource**>(ppvObject));
			return S_OK;
		}
	}
	return MF_E_UNSUPPORTED_SERVICE;
}
