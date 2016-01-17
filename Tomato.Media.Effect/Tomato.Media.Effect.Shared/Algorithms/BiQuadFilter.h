//
// Tomato Media Effect
// BiQuad 滤波器
// 作者：SunnyCase
// 创建时间：2016-01-16
//
#pragma once
#include "common.h"
#include <cmath>

DEFINE_NS_MEDIA_EFFECT

template<typename T = float>
struct BiQuadFilter
{
	T A0, A1, A2;
	T B1, B2;
	T Fc, Q;
	T GainDB;
	T Z1, Z2;

	BiQuadFilter()
	{
		memset(this, 0, sizeof(BiQuadFilter));
	}

	T Process(T input)
	{
		auto o = input * A0 + Z1;
		Z1 = input * A1 + Z2 - B1 * o;
		Z2 = input * A2 - B2 * o;
		return o;
	}
	
	void CalculateBiQuadCoefficients()
	{
		T norm;
		auto v = std::pow(T(10), std::abs(GainDB) / T(20.0));
		auto k = std::tan(T(3.1415926) * Fc);
		if (GainDB >= 0)
		{
			norm = T(1) / (T(1) + T(1) / Q * k + k * k);
			A0 = (T(1) + v / Q * k + k * k) * norm;
			A1 = T(2) * (k * k - 1) * norm;
			A2 = (T(1) - v / Q * k + k * k) * norm;
			B1 = A1;
			B2 = (T(1) - T(1) / Q * k + k * k) * norm;
		}
		else
		{
			norm = T(1) / (T(1) + v / Q * k + k * k);
			A0 = (T(1) + T(1) / Q * k + k * k) * norm;
			A1 = T(2) * (k * k - 1) * norm;
			A2 = (T(1) - T(1) / Q * k + k * k) * norm;
			B1 = A1;
			B2 = (T(1) - v / Q * k + k * k) * norm;
		}
	}
};

END_NS_MEDIA_EFFECT