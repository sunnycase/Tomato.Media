//
// Tomato Media
// Windows.Media.Core.IMediaSource 实现
// 
// (c) SunnyCase 
// 创建日期 2015-05-07
#include "pch.h"
#include "../../include/win32_exception.h"
#include "WindowsCoreMediaSource.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace Windows::Storage::Streams;
using namespace wrl;
using namespace concurrency;

WindowsCoreMediaSource::WindowsCoreMediaSource()
{
}

STDMETHODIMP WindowsCoreMediaSource::GetService(REFGUID guidService, REFIID riid, LPVOID * ppvObject)
{
	try
	{
		if (ppvObject)
			return E_POINTER;

		if (guidService == MF_MEDIASOURCE_SERVICE)
		{
			if (riid != IID_IMFMediaSource)
				return E_INVALIDARG;

			*ppvObject = OnCreateMediaSource().Detach();
			return S_OK;
		}
		else
			// 不支持的服务
			return MF_E_UNSUPPORTED_SERVICE;
	}
	CATCH_ALL();
}

ComPtr<IMFMediaSource> WindowsCoreMediaSource::OnCreateMediaSource()
{
	ComPtr<IMFSourceResolver> sourceResolver;
	THROW_IF_FAILED(MFCreateSourceResolver(&sourceResolver));

	//sourceResolver->CreateObjectFromURL()
}