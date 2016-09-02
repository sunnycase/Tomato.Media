//
// Tomato Media Toolkit
// Media Environment
// 
// 作者：SunnyCase
// 创建时间：2016-09-02
#pragma once
#include "common.h"
#include <Tomato.Media/CodecManager.h>

DEFINE_NS_MEDIA_TOOLKIT

public ref class MediaEnvironment sealed
{
public:
	MediaEnvironment();

	void RegisterDefaultCodecs();
private:
	Codec::CodecManager _codecManager;
};

END_NS_MEDIA_TOOLKIT