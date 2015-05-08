//
// Tomato Media
// Media Sink
// 
// (c) SunnyCase 
// 创建日期 2015-03-14
#pragma once
#include "platform.h"

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
	Stopped,
	// 媒体结束
	Ended
};

// Media Sink
class IMediaSink
{
public:
	IMediaSink() {}
	virtual ~IMediaSink() {}

	// 加载
	virtual concurrency::task<void> Initialize() = 0;
	// 设置状态改变回调
	virtual void SetStateChangedCallback(std::function<void(MediaSinkState)> callback) = 0;
	// 设置时间改变回调
	virtual void SetTimeChangedCallback(std::function<void(int64_t)> callback) = 0;
	// 设置媒体源读取器
	virtual concurrency::task<void> SetMediaSourceReader(std::shared_ptr<ISourceReader> sourceReader) = 0;
	// 开始播放
	// hns == -1 时继续播放
	virtual void StartPlayback(int64_t hns = -1) = 0;
	// 暂停
	virtual void PausePlayback() = 0;
	// 停止
	virtual void StopPlayback() = 0;
	// 获取当前时间
	virtual int64_t GetCurrentTime() = 0;
	// 获取音量
	virtual double GetVolume() = 0;
	// 设置音量
	virtual void SetVolume(double volume) = 0;
};

MEDIA_CORE_API std::unique_ptr<IMediaSink> __stdcall CreateWASAPIMediaSink();

NSED_TOMATO_MEDIA