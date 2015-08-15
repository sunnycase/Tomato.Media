//
// Tomato Media
// 默认视频 Shader 公用
// 作者：SunnyCase
// 创建时间：2015-08-15
//

struct VertexInput
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXTCOORD;
};

struct PixelInput
{
	float4 Position : SV_Position;
	float4 Color : COLOR0;
};

struct PixelOutput
{
	float4 Color : SV_Target;
};