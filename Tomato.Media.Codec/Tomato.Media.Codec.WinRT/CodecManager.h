//
// Tomato Media Codec
// Codec 管理器
// 
// 作者：SunnyCase
// 创建时间：2015-09-06
#pragma once
#include "common.h"
#include "Tomato.Media.Codec_i.h"
#include <windows.media.core.h>

DEFINE_NS_MEDIA_CODEC

class CodecManager : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::WinRt>, WRL::FtmBase,
	ABI::NS_MEDIA_CODEC::ICodecManager>
{
	InspectableClass(RuntimeClass_Tomato_Media_Codec_CodecManager, BaseTrust);
public:
	HRESULT RuntimeClassInitialize();
	// 通过 RuntimeClass 继承
	STDMETHODIMP RegisterDefaultCodecs(void) override;
private:
	WRL::ComPtr<ABI::Windows::Media::IMediaExtensionManager> mediaExtensionManager;
};

END_NS_MEDIA_CODEC