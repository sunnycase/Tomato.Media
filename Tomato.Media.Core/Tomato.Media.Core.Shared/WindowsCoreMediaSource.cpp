//
// Tomato Media
// Windows.Media.Core.IMediaSource 实现
// 
// (c) SunnyCase 
// 创建日期 2015-05-07
#include "pch.h"
#include "../include/win32_exception.h"
#include "../include/WindowsCoreMediaSource.h"
#include "MediaSources/IMediaSourceIntern.h"
#include "SourceReaders/MFSourceReader.h"

using namespace NS_TOMATO;
using namespace NS_TOMATO_MEDIA;
using namespace Windows::Storage::Streams;
using namespace wrl;
using namespace concurrency;

namespace
{
	class WindowsCoreMediaSource sealed : public RuntimeClass<RuntimeClassFlags<
		RuntimeClassType::WinRtClassicComMix>, ABI::Windows::Media::Core::IMediaSource, IMFGetService>
	{
	public:
		WindowsCoreMediaSource(const std::wstring& url)
			:url(url)
		{
			RegisterMFTs();

			THROW_IF_FAILED(MFCreateSourceResolver(&sourceResolver));
		}

		// 通过 RuntimeClass 继承
		STDMETHOD(GetService)(REFGUID guidService, REFIID riid, LPVOID * ppvObject) override
		{
			if (!ppvObject)
				return E_POINTER;

			if (guidService == MF_MEDIASOURCE_SERVICE)
			{
				if (riid == IID_IMFMediaSource)
				{
					try
					{
						ComPtr<IUnknown> mediaSourceUnk;
						MF_OBJECT_TYPE objType;
						THROW_IF_FAILED(sourceResolver->CreateObjectFromURL(
							url.c_str(), MF_RESOLUTION_MEDIASOURCE, nullptr, &objType, &mediaSourceUnk));
						THROW_IF_NOT(objType == MF_OBJECT_MEDIASOURCE, "Cannot create MFMediaSource.");
						
						ComPtr<IMFMediaSource> mediaSource;
						THROW_IF_FAILED(mediaSourceUnk.As(&mediaSource));
						*ppvObject = mediaSource.Detach();
						return S_OK;
					}
					CATCH_ALL();
				}
				return E_INVALIDARG;
			}
			// 不支持的服务
			return MF_E_UNSUPPORTED_SERVICE;
		}
	private:
		ComPtr<IMFSourceResolver> sourceResolver;
		ComPtr<IMFByteStream> byteStream;
		std::wstring url;
	};
}

Windows::Media::Core::IMediaSource^ __stdcall NS_TOMATO_MEDIA::CreateWindowsCoreMediaSource(const std::wstring& url)
{
	return reinterpret_cast<Windows::Media::Core::IMediaSource^>(
		Make<WindowsCoreMediaSource>(url).Detach());
}