//
// Tomato Media
// 后台音频进程间通信
// 
// (c) SunnyCase 
// 创建日期 2015-05-11

namespace Tomato
{
	namespace Media
	{
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
			TValue GetRestSetting()
			{
				auto values = GetSettingContainer()->Values;
				auto value = static_cast<TValue>(values->Lookup(TSetting::Key));
				values->Remove(TSetting::Key);
				return value;
			}

			namespace settings
			{
				struct HandlerFullNameSetting
				{
					static Platform::String^ Key;
					using TValue = Platform::String^;
				};
			}
		}
	}
}