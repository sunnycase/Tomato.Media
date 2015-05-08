//
// Tomato Media
// 媒体源读取器
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "platform.h"

NSDEF_TOMATO_MEDIA

class IMediaSource;
// 媒体源读取器状态
enum class SourceReaderState
{
	// 未加载
	NotInitialized,
	// 加载中
	Initializing,
	// 就绪
	Ready,
	// 缓冲
	PreRoll,
	// 正在播放
	Playing,
	// 已停止
	Stopped,
	// 媒体已结束
	Ended
};

// 媒体源读取器
class ISourceReader
{
public:
	ISourceReader() {}
	virtual ~ISourceReader() {}

	// 开始读取
	virtual void Start(int64_t hns = 0) = 0;
	// 停止
	virtual concurrency::task<void> StopAsync() = 0;
	// 设置音频格式
	virtual void SetAudioFormat(const WAVEFORMATEX* format, uint32_t framesPerPeriod) = 0;
	// 读取数据
	virtual size_t Read(byte* buffer, size_t bufferSize) = 0;
	// 获取状态
	virtual SourceReaderState GetState() const = 0;
	// 设置当前时间
	virtual void SetCurrentPosition(int64_t hns) = 0;
	// 获取缓冲起始时间
	virtual int64_t GetBufferStartPosition() const noexcept = 0;
};

MEDIA_CORE_API std::unique_ptr<ISourceReader> __stdcall CreateMFSourceReader(IMediaSource* mediaSource);
MEDIA_CORE_API void RegisterMFTs();

NSED_TOMATO_MEDIA