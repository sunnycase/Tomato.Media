//
// Tomato Media
// Media Engine Notify 实现
// 作者：SunnyCase
// 创建时间：2015-08-06
//
#include "pch.h"
#include "MediaEngineNotify.h"

using namespace NS_MEDIA;
using namespace concurrency;
using namespace WRL;

#if (WINVER >= _WIN32_WINNT_WIN8)

MediaEngineNotify::MediaEngineNotify(std::function<void(DWORD)>&& eventCallback)
	:eventCallback(std::move(eventCallback))
{

}

HRESULT MediaEngineNotify::EventNotify(DWORD event, DWORD_PTR param1, DWORD param2)
{
	if (event == MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE)
	{
		SetEvent(reinterpret_cast<HANDLE>(param1));
	}
	else if(eventCallback)
	{
		eventCallback(event);
	}

	return S_OK;
}

#endif