//
// Tomato Media
// 默认视频 Vertex Shader
// 作者：SunnyCase
// 创建时间：2015-08-15
//
#include "DefaultVideoShader.hlsli"

PixelInput main(VertexInput input)
{
	PixelInput output;
	output.Position = input.Position;
	output.Color = float4(1.0f, 0.0f, 1.0f, 1.0f);

	return output;
}