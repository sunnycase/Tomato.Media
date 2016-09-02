//
// Tomato Media Toolkit
// Media Environment
// 
// 作者：SunnyCase
// 创建时间：2016-09-02
#include "pch.h"
#include "MediaEnvironment.h"

using namespace NS_MEDIA_TOOLKIT;
using namespace NS_MEDIA_CODEC;

MediaEnvironment::MediaEnvironment()
{

}

void MediaEnvironment::RegisterDefaultCodecs()
{
	_codecManager.RegisterDefaultCodecs();
}
