//
// Tomato Media
// Media Engine Notify 实现
// 作者：SunnyCase
// 创建时间：2015-08-06
//
#pragma once
#include "common.h"
#include <Mfmediaengine.h>

DEFINE_NS_MEDIA

#if (WINVER >= _WIN32_WINNT_WIN8)

class MediaEngineNotify : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::RuntimeClassType::ClassicCom>, IMFMediaEngineNotify>
{
public:
	MediaEngineNotify(std::function<void(DWORD)>&& eventCallback);

	// 通过 RuntimeClass 继承
	STDMETHODIMP EventNotify(DWORD event, DWORD_PTR param1, DWORD param2) override;
private:
	std::function<void(DWORD)> eventCallback;
};

#endif

END_NS_MEDIA