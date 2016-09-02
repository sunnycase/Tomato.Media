//
// Tomato Media Codec
// Codec 管理
// 
// 作者：SunnyCase
// 创建时间：2016-09-02
#pragma once
#include "Tomato.Media.h"
#include <windows.media.core.h>

DEFINE_NS_MEDIA_CODEC

#pragma warning(push)
#pragma warning(disable: 4251)

class TOMATO_MEDIA_API CodecManager final
{
public:
	CodecManager();

	void RegisterDefaultCodecs();
private:
	WRL::ComPtr<ABI::Windows::Media::IMediaExtensionManager> _mediaExtensionManager;
};

#pragma warning(pop)

END_NS_MEDIA_CODEC