//
// Tomato Media
// 生命周期
// 作者：SunnyCase
// 创建时间：2015-08-04
//
#include "pch.h"
#include "common.h"

namespace
{
	struct LifeTime
	{
		LifeTime()
		{
			auto hr = MFStartup(MF_SDK_VERSION);
			if (FAILED(hr))
				_com_issue_error(hr);
		}

		~LifeTime()
		{
			MFShutdown();
		}
	} lifeTime;
}