//
// Tomato Media
// 音频渲染器接口
// 
// 作者：SunnyCase 
// 创建日期 2015-09-04
#pragma once
#include "common.h"
#include <ppltasks.h>
#include "../../include/MFWorkerQueueProvider.h"
#include <functional>

DEFINE_NS_MEDIA
///<summary>音频渲染器接口</summary>
struct DECLSPEC_UUID("9DB108BD-070C-4D5D-8C46-25C3CD782488") IAudioRender : public IUnknown
{
	virtual concurrency::task<void> Initialize() = 0;
	virtual void SetWorkerQueueProvider(Core::MFWorkerQueueProviderRef workerQueue) = 0;
	virtual void SetIsActive(bool value) = 0;
	// 设置提供采样的回调
	// 回调类型：size_t(byte* buffer, size_t bytesToRead) 返回实际读取的字节数
	// 要求：非阻塞
	virtual void SetProvideSampleCallback(std::function<size_t(byte*, size_t)>&& callback) = 0;
};

END_NS_MEDIA

// {9DB108BD-070C-4D5D-8C46-25C3CD782488}
EXTERN_GUID(MF_TM_AUDIORENDER_SERVICE,
	0x9db108bd, 0x70c, 0x4d5d, 0x8c, 0x46, 0x25, 0xc3, 0xcd, 0x78, 0x24, 0x88);