//
// Tomato Media Effect
// 均衡器
// 作者：SunnyCase
// 创建时间：2016-01-17
//
#pragma once
#include "common.h"
#include "BiQuadFilter.h"
#include <vector>

DEFINE_NS_MEDIA_EFFECT

class EqualizerFilter
{
public:
	EqualizerFilter(uint32_t sampleRate, float centerFreq, float bandWidth, float gain);

	float Process(float input);
private:
	BiQuadFilter<float> _filter;
};

constexpr struct TenBandEqulizer_t
{

} tenBandEqualizer;

class MultiBandEqualizerFilter
{
public:
	MultiBandEqualizerFilter(uint32_t sampleRate);
	MultiBandEqualizerFilter(TenBandEqulizer_t tenBand, uint32_t sampleRate,float bandWidth = 18.f, float gain = 0.f);

	void Add(float centerFreq, float bandWidth, float gain);
	float Process(float input);
private:
	std::vector<EqualizerFilter> _filters;
	const uint32_t _sampleRate;
};

END_NS_MEDIA_EFFECT