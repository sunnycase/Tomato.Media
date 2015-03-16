//
// Tomato Media
// Media Sink
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#pragma once
#include "tomato.media.core.h"

NSDEF_TOMATO_MEDIA

class ISourceReader;
// Media Sink 状态
enum class MediaSinkState
{
	// 未加载
	NotInitialized,
	// 加载中
	Initializing,
	// 就绪
	Ready,
	// 开始播放
	StartPlaying,
	// 正在播放
	Playing,
	// 正在暂停
	Pausing,
	// 已暂停
	Paused,
	// 正在停止
	Stopping,
	// 已停止
	Stopped
};

// Media Sink
class IMediaSink
{
public:
	IMediaSink() {}
	virtual ~IMediaSink() {}

	// 加载
	virtual concurrency::task<void> Initialize() = 0;
	// 设置媒体源读取器
	virtual void SetMediaSourceReader(std::shared_ptr<ISourceReader> sourceReader) = 0;
	// 开始播放
	virtual void StartPlayback() = 0;
};

MEDIA_CORE_API std::unique_ptr<IMediaSink> __stdcall CreateWASAPIMediaSink();

NSED_TOMATO_MEDIA