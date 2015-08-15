//
// Tomato Media
// 默认视频 Pixel Shader
// 作者：SunnyCase
// 创建时间：2015-08-15
//
#include "DefaultVideoShader.hlsli"

Texture2D Luminance;
Texture2D Chrominance;

SamplerState TextureSampler;

PixelOutput main(PixelInput input)
{
	PixelOutput output;

	float Yf = Luminance.Sample(TextureSampler, input.TexCoord).r;
	float2 UVf = Chrominance.Sample(TextureSampler, input.TexCoord).rg;
	float Y = Yf;
	float U = UVf.r;
	float V = UVf.g;

	int C = Y * 255 - 16;
	int D = U * 255 - 128;
	int E = V * 255 - 128;

	float R = saturate((298 * C + 409 * E + 128) / 65536.f);
	float G = saturate((298 * C - 100 * D - 208 * E + 128) / 65536.f);
	float B = saturate((298 * C + 516 * D + 128) / 65536.f);
	output.Color = float4(R, G, B, 1.0f);

	return output;
}