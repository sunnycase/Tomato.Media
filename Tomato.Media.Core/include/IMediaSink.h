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
enum class MediaSinkStatus
{
	// 已关闭
	Closed,
	// 状态正在改变
	Changing,
	// 已经开始
	Started,
	// 已暂停
	Paused,
	// 已停止
	Stopped
};

// Media Sink 处理器
struct IMediaSinkHandler
{
	virtual void OnStatusChanged(MediaSinkStatus status) = 0;
	virtual void OnMediaOpened() = 0;
	virtual void OnMediaEnded() = 0;
	virtual void OnSeekCompleted() = 0;
	virtual void OnError(HRESULT error) = 0;
};

///<summary>Media Sink</summary>
///<remark>
/// Media Sink 对象模型：
/// Media Sink 基于状态机实现。
/// 初始化后 CurrentStatus 设置为 Closed。
/// 1. 调用 SetMediaSourceReader 设置媒体源，设置成功 CurrentStatus 设置为 Paused，
///    产生 MediaOpened 事件。进入步骤 2 或 5。
/// 2. 调用 Start，CurrentStatus 设置为 Changing，
///    开始播放后 CurrentStatus 设置为 Started。可进入步骤 3 或 4、5、6。
/// 3. 调用 Pause，CurrentStatus 设置为 Changing，
///    暂停后 CurrentStatus 设置为 Paused。可进入步骤 2 或 4、5。
/// 4. 调用 Stop，CurrentStatus 设置为 Changing，
///    停止后 CurrentStatus 设置为 Stopped。可进入步骤 1 或 2、5。
/// 5. 设置 Position 调整播放位置，设置成功产生 SeekCompleted 事件。
/// 6. 媒体自然结束后产生 MediaEnded 事件，CurrentStatus 设置为 Paused。
///    可进入步骤 1 或设置 非 Duration 的 Position 后进入 2。
/// 7. 以上步骤产生错误均会产生 Error 事件。
///</remark>
class IMediaSink
{
public:
	// Media Sink 不控制 IMediaSinkHandler 的生存期，
	// 调用者需保证 IMediaSinkHandler 的生存期大于 Media Sink 的生存期。
	IMediaSink(IMediaSinkHandler&) {}
	virtual ~IMediaSink() {}

	// 设置媒体源读取器
	virtual void SetMediaSourceReader(std::shared_ptr<ISourceReader> sourceReader) = 0;
	// 获取当前状态
	virtual MediaSinkStatus GetCurrentStatus() const noexcept = 0;
	// 开始
	virtual void Start() = 0;
	// 暂停
	virtual void Pause() = 0;
	// 停止
	virtual void Stop() = 0;
	// 获取位置
	virtual int64_t GetPosition() const = 0;
	// 设置位置
	virtual void SetPosition(int64_t position) = 0;
	// 获取总长度
	virtual int64_t GetDuration() const = 0;
	// 获取音量
	virtual double GetVolume() = 0;
	// 设置音量
	virtual void SetVolume(double volume) = 0;
};

// Media Sink 不控制 IMediaSinkHandler 的生存期，
// 调用者需保证 IMediaSinkHandler 的生存期大于 Media Sink 的生存期。
MEDIA_CORE_API std::unique_ptr<IMediaSink> __stdcall CreateWASAPIMediaSink(IMediaSinkHandler& handler);

NSED_TOMATO_MEDIA