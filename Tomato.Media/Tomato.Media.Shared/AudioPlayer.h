//
// Tomato Media
// 音频播放器
// 作者：SunnyCase
// 创建时间：2015-09-13
//
#pragma once
#include "common.h"

DEFINE_NS_MEDIA_INTERN

// 音频播放器
class AudioPlayer final : public std::enable_shared_from_this<AudioPlayer>
{
public:
	AudioPlayer();
};

END_NS_MEDIA_INTERN