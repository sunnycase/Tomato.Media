//
// Tomato Media
// Media Foundation Transform
// 
// (c) SunnyCase 
// 创建日期 2015-03-15
#pragma once
#include "../../../include/tomato.media.core.h"

NSDEF_TOMATO_MEDIA

enum class TransformState
{
	Initializing,
	Ready,
	WaitingInput,
	PendingOutput
};

NSED_TOMATO_MEDIA