#pragma once

#include "Containers/SeonString.h"
#include "Logging/LogCategory.h"
#include "Logging/LogVerbosity.h"

#include <format>

namespace SE::Private
{
	// 컴파일할 때 남길 가장 낮은 레벨이다. 이보다 낮은 레벨의 SE_LOG는 코드가 통째로 사라진다.
	// 배포용인 Release는 Error와 Fatal만 남긴다. Unreal COMPILED_IN_MINIMUM_VERBOSITY와 같은 역할이다
#if SE_BUILD_RELEASE
	inline constexpr ELogVerbosity CompiledMinVerbosity = ELogVerbosity::Error;
#else
	inline constexpr ELogVerbosity CompiledMinVerbosity = ELogVerbosity::VeryVerbose;
#endif

	/**
	 * 형식 문자열과 값 묶음으로 메시지를 만들어 전달기에 넘긴다.
	 * 템플릿이 아니라서 메시지를 만드는 코드가 로그를 쓰는 곳마다 생기지 않고 프로그램에 한 벌만 있다.
	 * 형식 검사는 이 함수를 부르는 쪽에서 이미 끝났다.
	 */
	void LogFormatted(const FLogCategory& category, ELogVerbosity verbosity, FStringView format, std::format_args args);

	/**
	 * SE_LOG가 부르는 얇은 템플릿이다. 형식 검사와 값 묶기만 하고 메시지 만들기는 LogFormatted에 넘긴다.
	 * format을 std::format_string으로 받아서, {} 개수와 값 타입이 맞지 않으면 컴파일 에러가 난다.
	 */
	template<typename... ArgTypes>
	void Log(const FLogCategory& category, ELogVerbosity verbosity, std::format_string<ArgTypes...> format, ArgTypes&&... args)
	{
		// 이름이 붙은 args는 변수라서, 임시 값을 받지 않는 make_format_args에 그대로 넘길 수 있다
		LogFormatted(category, verbosity, format.get(), std::make_format_args(args...));
	}

	/** Fatal 로그를 남기고 프로그램을 비정상 종료한다. 돌아오지 않는다. */
	[[noreturn]] void LogFatalFormatted(const FLogCategory& category, FStringView format, std::format_args args);

	/** SE_LOG의 Fatal 경로가 부르는 얇은 템플릿이다. Log와 같고, 레벨은 항상 Fatal이다. */
	template<typename... ArgTypes>
	[[noreturn]] void LogFatal(const FLogCategory& category, std::format_string<ArgTypes...> format, ArgTypes&&... args)
	{
		LogFatalFormatted(category, format.get(), std::make_format_args(args...));
	}
}

// 로그를 남긴다. 예: SE_LOG(LogWindows, Error, "RegisterClassExW failed (error {})", GetLastError());
// 함수가 아니라 매크로인 이유: 레벨이 꺼져 있으면 인자 계산까지 건너뛰고, 컴파일 기준보다 낮은 레벨은 코드를 없앤다.
// Fatal은 맨 앞에서 갈라져 카테고리 검사와 컴파일 기준을 거치지 않는다. 카테고리를 꺼도 원인을 남기고 종료한다.
// do { } while (0)으로 감싸 if / else 안에서도 문장 하나처럼 쓸 수 있게 한다.
// TODO(seon): 디버거 확인 함수를 만들면 Fatal 분기에서 종료 전에 디버거가 붙어 있으면 이 줄에서 멈춘다
#define SE_LOG(CategoryName, Verbosity, Format, ...) \
	do \
	{ \
		static_assert(ELogVerbosity::Verbosity != ELogVerbosity::NoLogging, "NoLogging은 카테고리 기준 레벨로만 쓴다"); \
		if constexpr (ELogVerbosity::Verbosity == ELogVerbosity::Fatal) \
		{ \
			SE::Private::LogFatal(CategoryName, Format __VA_OPT__(, ) __VA_ARGS__); \
		} \
		else if constexpr (ELogVerbosity::Verbosity <= SE::Private::CompiledMinVerbosity) \
		{ \
			if (CategoryName.IsEnabled(ELogVerbosity::Verbosity)) \
			{ \
				SE::Private::Log(CategoryName, ELogVerbosity::Verbosity, Format __VA_OPT__(, ) __VA_ARGS__); \
			} \
		} \
	} while (0)
