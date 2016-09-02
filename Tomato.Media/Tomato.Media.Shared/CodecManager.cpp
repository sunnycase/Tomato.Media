//
// Tomato Media Codec
// Codec 管理
// 
// 作者：SunnyCase
// 创建时间：2016-09-02
#include "pch.h"
#include <Tomato.Media/Tomato.Media.h>
#include <Tomato.Media/CodecManager.h>
#include "ByteStreamHandlers/FFmpegByteStreamHandler.h"
#include "Transforms/FFmpegAudioDecoderTransform.h"

using namespace NS_MEDIA_CODEC;
using namespace WRL;
using namespace Windows::Foundation;

template<class T>
void RegisterByteStreamHandler(ABI::Windows::Media::IMediaExtensionManager* mediaExtensionManager)
{
	Wrappers::HStringReference className(T::InternalGetRuntimeClassName());
	for (auto&& item : T::RegisterInfos)
	{
		Wrappers::HStringReference fileExtension(item.FileExtension);
		Wrappers::HStringReference mimeType(item.MimeType);

		ThrowIfFailed(mediaExtensionManager->RegisterByteStreamHandler(className.Get(),
			fileExtension.Get(), mimeType.Get()));
	}
}

template<class T>
void RegisterVideoDecoderTransform(ABI::Windows::Media::IMediaExtensionManager* mediaExtensionManager)
{
	Wrappers::HStringReference className(T::InternalGetRuntimeClassName());
	for (auto&& item : T::RegisterInfos)
	{
		ThrowIfFailed(mediaExtensionManager->RegisterVideoDecoder(className.Get(),
			item.InputSubType, item.OutputSubType));
	}
}

template<class T>
void RegisterAudioDecoderTransform(ABI::Windows::Media::IMediaExtensionManager* mediaExtensionManager)
{
	Wrappers::HStringReference className(T::InternalGetRuntimeClassName());
	for (auto&& item : T::RegisterInfos)
	{
		ThrowIfFailed(mediaExtensionManager->RegisterAudioDecoder(className.Get(),
			item.InputSubType, item.OutputSubType));
	}
}

CodecManager::CodecManager()
{
	Wrappers::HStringReference mediaExtensionManagerClassName(RuntimeClass_Windows_Media_MediaExtensionManager);
	ThrowIfFailed(ActivateInstance(mediaExtensionManagerClassName.Get(), &_mediaExtensionManager));
}

void CodecManager::RegisterDefaultCodecs()
{
	RegisterByteStreamHandler<FFmpegByteStreamHandler>(_mediaExtensionManager.Get());
	RegisterAudioDecoderTransform<FFmpegAudioDecoderTransform>(_mediaExtensionManager.Get());
}
