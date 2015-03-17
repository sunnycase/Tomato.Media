//
// Tomato Music 媒体
// Media Foundation 媒体源操作
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#pragma once
#include "../include/tomato.media.core.h"

NSDEF_TOMATO_MEDIA

// 媒体源操作类型
enum class MediaSourceOperationKind
{
	// 无
	None,
	// 开始
	Start,
	// 暂停
	Pause,
	// 停止
	Stop,
	// 设置速率
	SetRate,
	// 请求数据
	RequestData,
	// 到达结尾
	EndOfStream
};

// 媒体源操作
class MediaSourceOperation
{
public:
	MediaSourceOperation(MediaSourceOperationKind kind) noexcept
		: kind(kind)
	{

	}

	MediaSourceOperation() noexcept
		: MediaSourceOperation(MediaSourceOperationKind::None)
	{

	}

	// 获取类型
	MediaSourceOperationKind GetKind() const noexcept { return kind; }
private:
	MediaSourceOperationKind kind;
};

class MediaSourceStartOperation : public MediaSourceOperation
{
public:
	MediaSourceStartOperation(wrl::ComPtr<IMFPresentationDescriptor> pd, REFERENCE_TIME position) noexcept
		: MediaSourceOperation(MediaSourceOperationKind::Start),
		pd(pd), position(position)
	{

	}

	IMFPresentationDescriptor* GetPresentationDescriptor() const noexcept
	{
		return pd.Get();
	}

	REFERENCE_TIME GetPosition() const noexcept
	{
		return position;
	}
private:
	wrl::ComPtr<IMFPresentationDescriptor> pd;
	REFERENCE_TIME position;
};

NSED_TOMATO_MEDIA