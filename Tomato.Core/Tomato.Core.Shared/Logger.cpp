//
// Tomato Media Core
// 日志记录器
// 作者：SunnyCase
// 创建时间：2015-09-05
//
#include "pch.h"
#include "Logger.h"
#include "LoggingConfiguration.h"

using namespace NS_CORE_INTERN;

Logger::Logger(std::wstring typeName)
	:typeName(std::move(typeName))
{
}

namespace
{
	std::wstring time_point_to_string(const std::chrono::system_clock::time_point &tp)
	{
		using namespace std;
		using namespace std::chrono;

		auto ttime_t = system_clock::to_time_t(tp);
		auto tp_sec = system_clock::from_time_t(ttime_t);
		milliseconds ms = duration_cast<milliseconds>(tp - tp_sec);

		std::tm ttm{ 0 };
		if (localtime_s(&ttm, &ttime_t) == 0)
		{
			wchar_t date_time_format[] = L"%Y.%m.%d-%H.%M.%S";

			wchar_t time_str[] = L"yyyy.mm.dd.HH-MM.SS.fff";

			wcsftime(time_str, ARRAYSIZE(time_str), date_time_format, &ttm);

			wstring result(time_str);
			result.append(L".");
			result.append(to_wstring(ms.count()));

			return result;
		}
		return L"";
	}
}

void Logger::Information(const std::wstring & message)
{
	auto time = std::chrono::system_clock::now();
	std::wstringstream ss;
	ss << time_point_to_string(time) << '\t' << typeName << "\t" << message << std::endl;
	auto str = ss.str();
	for (auto&& appender : LoggingConfiguration::Current().GetAppenders())
		appender->Information(str);
}
