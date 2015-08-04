//
// Tomato Media
// 应用程序数据辅助
// 
// 作者：SunnyCase 
// 创建日期 2015-08-04
#pragma once
#include "common.h"

DEFINE_NS_MEDIA

namespace details
{
	Windows::Storage::ApplicationDataContainer^ GetSettingContainer();

	template<typename TSetting, typename TValue = TSetting::TValue>
	// 设置数据
	void SetSetting(TValue value)
	{
		GetSettingContainer()->Values->Insert(TSetting::Key, value);
	}

	template<typename TSetting, typename TValue = TSetting::TValue>
	// 读取并清除数据
	TValue GetResetSetting()
	{
		auto values = GetSettingContainer()->Values;
		auto value = static_cast<TValue>(values->Lookup(TSetting::Key));
		values->Remove(TSetting::Key);
		return value;
	}

	namespace settings
	{
		struct BackgroundMediaPlayerHandlerFullNameSetting
		{
			static Platform::String^ Key;
			using TValue = Platform::String^;
		};
	}
}

END_NS_MEDIA