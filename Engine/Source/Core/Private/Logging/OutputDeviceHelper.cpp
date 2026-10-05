#include "Logging/OutputDeviceHelper.h"

#include <chrono>
#include <format>

FString FOutputDeviceHelper::FormatLogLine(const FLogRecord& record)
{
	// %S는 시각의 정밀도대로 소수점 아래까지 찍는다(MSVC system_clock은 100ns 단위라 "56.7891234").
	// Unreal 모양("56:789")으로 쓰려고 초 단위로 자른 시각과 남은 밀리초를 나눠 쓴다
	const std::chrono::sys_seconds seconds = std::chrono::floor<std::chrono::seconds>(record.time);
	const int64 milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(record.time - seconds).count();

	FString line = std::format("[{:%Y.%m.%d-%H.%M.%S}:{:03}]{}: ", seconds, milliseconds, record.category.name);
	if (record.verbosity != ELogVerbosity::Log)
	{
		line += ToString(record.verbosity);
		line += ": ";
	}
	line += record.message;
	return line;
}
