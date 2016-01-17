//
// Tomato Media
// 均衡器
// 作者：SunnyCase
// 创建时间：2016-01-17
//
#include "pch.h"
#include "EqualizerFilter.h"

using namespace NS_MEDIA;
using namespace NS_MEDIA_EFFECT;

EqualizerFilter::EqualizerFilter(uint32_t sampleRate, float centerFreq, float bandWidth, float gain)
{
	ThrowIfNot(sampleRate >= centerFreq * 2, L"The sampleRate has to be bigger than 2 * frequency.");
	_filter.Q = bandWidth;
	_filter.Fc = centerFreq / sampleRate;
	_filter.GainDB = gain;

	_filter.CalculateBiQuadCoefficients();
}

float EqualizerFilter::Process(float input)
{
	return _filter.Process(input);
}

MultiBandEqualizerFilter::MultiBandEqualizerFilter(uint32_t sampleRate)
	:_sampleRate(sampleRate)
{
}

MultiBandEqualizerFilter::MultiBandEqualizerFilter(TenBandEqulizer_t tenBand, uint32_t sampleRate,float bandWidth, float gain)
	: MultiBandEqualizerFilter(sampleRate)
{
	Add(31, bandWidth, gain);
	Add(62, bandWidth, gain);
	Add(125, bandWidth, gain);
	Add(250, bandWidth, gain);
	Add(500, bandWidth, gain);
	Add(1000, bandWidth, gain);
	Add(2000, bandWidth, gain);
	Add(4000, bandWidth, gain);
	Add(8000, bandWidth, gain);
	Add(16000, bandWidth, gain);
}

void MultiBandEqualizerFilter::Add(float centerFreq, float bandWidth, float gain)
{
	_filters.emplace_back(_sampleRate, centerFreq, bandWidth, gain);
}

float MultiBandEqualizerFilter::Process(float input)
{
	for (auto&& filter : _filters)
		input = filter.Process(input);
	return input;
}
