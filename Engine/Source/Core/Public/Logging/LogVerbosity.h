#pragma once

#include "CoreTypes.h"

// 로그 레벨이다. 심각할수록 값이 작아서, "이 레벨까지 출력"을 level <= 기준 레벨 한 번으로 비교한다.
enum class ELogVerbosity : uint8
{
	// 카테고리의 기준 레벨로만 쓴다. 이 카테고리의 로그를 모두 끈다(Fatal은 예외로 항상 출력한다).
	// "값이 없음"이 아니라 "로그 끔"이라는 뜻이라 None 대신 이 이름을 쓴다
	NoLogging,
	// 복구할 수 없는 오류. 로그를 남기고 종료한다
	Fatal,
	// 기능이 실패함
	Error,
	// 동작은 하지만 문제가 있음
	Warning,
	// 콘솔과 로그 파일에 모두 남길 주요 정보
	Display,
	// 로그 파일에만 남길 일반 정보
	Log,
	// 자세한 디버깅 정보. 카테고리에서 켰을 때만 출력한다
	Verbose,
	// 매 프레임 출력처럼 매우 자세한 정보. 카테고리에서 켰을 때만 출력한다
	VeryVerbose
};

// 레벨 이름을 글자로 돌려준다. 열거형 값은 숫자라서 로그 줄에 "Warning" 같은 이름을 쓰려면 직접 바꿔야 한다.
// 문자열 리터럴을 돌려주므로 프로그램이 끝날 때까지 유효하다.
constexpr const char* ToString(ELogVerbosity verbosity)
{
	switch (verbosity)
	{
		case ELogVerbosity::NoLogging:
			return "NoLogging";
		case ELogVerbosity::Fatal:
			return "Fatal";
		case ELogVerbosity::Error:
			return "Error";
		case ELogVerbosity::Warning:
			return "Warning";
		case ELogVerbosity::Display:
			return "Display";
		case ELogVerbosity::Log:
			return "Log";
		case ELogVerbosity::Verbose:
			return "Verbose";
		case ELogVerbosity::VeryVerbose:
			return "VeryVerbose";
	}
	// static_cast로 목록에 없는 값이 들어와도 무언가를 돌려준다
	return "Unknown";
}
