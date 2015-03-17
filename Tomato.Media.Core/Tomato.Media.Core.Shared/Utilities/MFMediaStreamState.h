//
// Tomato Media
// Media Foundation 媒体流状态
// 
// (c) SunnyCase 
// 创建日期 2015-03-17
#pragma once
#include "../include/tomato.media.core.h"

NSDEF_TOMATO_MEDIA

// Media Foundation 媒体流状态
enum class MFMediaStreamState
{
	// 未加载
	NotInitialized,
	// 正在打开
	Opening,
	// 已停止
	Stopped,
	// 已暂停
	Paused,
	// 已开始
	Started,
	// 结尾
	EndOfStream
};

NSED_TOMATO_MEDIA