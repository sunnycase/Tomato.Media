//
// Tomato Media
// 媒体源读取器
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "tomato.media.core.h"

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
	// 已暂停
	Paused,
	// 已停止
	Stopped
};

// 媒体源读取器
class ISourceReader
{
public:
	ISourceReader() {}
	virtual ~ISourceReader() {}

	// 开始读取
	virtual void Start() = 0;
	// 设置音频格式
	virtual void SetAudioFormat(const WAVEFORMATEX* format, uint32_t framesPerPeriod) = 0;
	// 读取数据
	virtual size_t Read(byte* buffer, size_t bufferSize) = 0;
};

MEDIA_CORE_API std::unique_ptr<ISourceReader> __stdcall CreateMFSourceReader(IMediaSource* mediaSource);

NSED_TOMATO_MEDIA