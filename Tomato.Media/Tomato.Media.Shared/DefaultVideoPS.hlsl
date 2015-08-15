//
// Tomato Media
// 默认视频 Pixel Shader
// 作者：SunnyCase
// 创建时间：2015-08-15
//
#include "DefaultVideoShader.hlsli"

PixelOutput main(PixelInput input)
{
	PixelOutput output;
	output.Color = float4(1.0f, 0.0f, 1.0f, 1.0f);

	return output;
}