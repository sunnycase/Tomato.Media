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

	template<typename TSetting>
	// 设置数据
	void SetSetting(typename TSetting::TValue value)
	{
		GetSettingContainer()->Values->Insert(TSetting::Key, value);
	}

	template<typename TSetting>
	// 读取并清除数据
	typename TSetting::TValue GetResetSetting()
	{
		auto values = GetSettingContainer()->Values;
		auto value = static_cast<TSetting::TValue>(values->Lookup(TSetting::Key));
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