//
// Tomato Media Codec
// Codec 管理器
// 
// 作者：SunnyCase
// 创建时间：2015-09-06
#include "pch.h"
#include "CodecManager.h"
#include "ByteStreamHandlers/OggByteStreamHandler.h"

using namespace NS_MEDIA_CODEC;
using namespace WRL;

ActivatableClass(CodecManager);

HRESULT CodecManager::RuntimeClassInitialize()
{
	try
	{
		Wrappers::HStringReference mediaExtensionManagerClassName(RuntimeClass_Windows_Media_MediaExtensionManager);
		ComPtr<IInspectable> mediaExtensionManagerUnk;
		ThrowIfFailed(RoActivateInstance(mediaExtensionManagerClassName.Get(), &mediaExtensionManagerUnk));
		ThrowIfFailed(mediaExtensionManagerUnk.As(&mediaExtensionManager));
	}
	CATCH_ALL();
	return S_OK;
}

template<class T>
void RegisterByteStreamHandler(ABI::Windows::Media::IMediaExtensionManager* mediaExtensionManager)
{
	Wrappers::HStringReference className(T::InternalGetRuntimeClassName());
	for (auto&& item : T::RegisterItems)
	{
		Wrappers::HStringReference fileExtension(item.FileExtension);
		Wrappers::HStringReference mimeType(item.MimeType);

		ThrowIfFailed(mediaExtensionManager->RegisterByteStreamHandler(className.Get(),
			fileExtension.Get(), mimeType.Get()));
	}
}

HRESULT CodecManager::RegisterDefaultCodecs(void)
{
	try
	{
		RegisterByteStreamHandler<OggByteStreamHandler>(mediaExtensionManager.Get());
	}
	CATCH_ALL();
	return S_OK;
}
