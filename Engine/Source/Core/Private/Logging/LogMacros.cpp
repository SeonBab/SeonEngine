#include "Logging/LogMacros.h"

#include "Logging/OutputDeviceRedirector.h"

// 이름 공간을 붙여 정의하면, 헤더 선언과 매개변수가 어긋났을 때 새 함수가 생기지 않고 컴파일 에러가 난다
void SE::Private::LogFormatted(const FLogCategory& category, ELogVerbosity verbosity, FStringView format, std::format_args args)
{
	const FString message = std::vformat(format, args);
	FOutputDeviceRedirector::Get().Write(category, verbosity, message);
}

void SE::Private::LogFatalFormatted(const FLogCategory& category, FStringView format, std::format_args args)
{
	LogFormatted(category, ELogVerbosity::Fatal, format, args);
	// 소멸자와 정리 함수를 부르지 않고 바로 끝낸다. 망가진 상태에서 정리 코드가 더 큰 문제를 일으키지 않게 한다
	std::abort();
}
