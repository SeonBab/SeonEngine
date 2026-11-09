#include "Log.h"

#include <Windows.h>

#include <iostream>

namespace
{
	LogLevel minimumLevel = LogLevel::Debug;

	const char* GetLevelName(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Debug:
			return "Debug";
		case LogLevel::Info:
			return "Info";
		case LogLevel::Warning:
			return "Warning";
		case LogLevel::Error:
			return "Error";
		default:
			return "Unknown";
		}
	}

	WORD GetLevelColor(LogLevel level, WORD originalAttributes)
	{
		WORD foreground = 0;

		switch (level)
		{
		case LogLevel::Debug:
			foreground = FOREGROUND_INTENSITY;
			break;
		case LogLevel::Warning:
			foreground = FOREGROUND_RED | FOREGROUND_GREEN
				| FOREGROUND_INTENSITY;
			break;
		case LogLevel::Error:
			foreground = FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;
		default:
			return originalAttributes;
		}

		// 기존 배경색과 나머지 속성은 유지하고 글자색만 바꾼다.
		constexpr WORD ForegroundMask = FOREGROUND_RED
			| FOREGROUND_GREEN | FOREGROUND_BLUE
			| FOREGROUND_INTENSITY;

		return static_cast<WORD>(
			(originalAttributes & ~ForegroundMask) | foreground);
	}
}

void Log::SetMinimumLevel(LogLevel level)
{
	minimumLevel = level;
}

bool Log::IsEnabled(LogLevel level)
{
	return level >= minimumLevel;
}

void Log::Write(
	LogLevel level,
	std::string_view category,
	std::string_view message)
{
	if (level < minimumLevel) { return; }

	const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo{};

	// 파일이나 파이프로 출력되거나 콘솔 속성 조회에 실패하면 색상 처리를 생략한다.
	const bool hasConsoleAttributes =
		GetConsoleScreenBufferInfo(output, &consoleInfo) != FALSE;

	// 색상을 바꾸기 전에 버퍼에 남은 글자를 실제로 출력한다.
	std::cout << '[' << category << "] " << std::flush;

	bool colorChanged = false;
	if (hasConsoleAttributes)
	{
		colorChanged = SetConsoleTextAttribute(
			output,
			GetLevelColor(level, consoleInfo.wAttributes)) != FALSE;
	}

	std::cout << '[' << GetLevelName(level) << ']' << std::flush;

	if (colorChanged)
	{
		SetConsoleTextAttribute(output, consoleInfo.wAttributes);
	}

	std::cout << ' ' << message << '\n' << std::flush;
}
