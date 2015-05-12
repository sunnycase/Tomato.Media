//
// Tomato Media
// 后台音频进程间通信
// 
// (c) SunnyCase 
// 创建日期 2015-05-11
#include "pch.h"
#include "BackgroundAudioIPC.h"

using namespace Platform;
using namespace Windows::Storage;
using namespace Tomato::Media;

String^ details::settings::HandlerFullNameSetting::Key = L"HandlerFullName";

ApplicationDataContainer ^ details::GetSettingContainer()
{
	static auto container = ApplicationData::Current->LocalSettings->CreateContainer(
		L"_BackgroundAudoSetting", ApplicationDataCreateDisposition::Always);
	return container;
}
