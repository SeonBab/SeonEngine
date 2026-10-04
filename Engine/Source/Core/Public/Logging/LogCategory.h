#pragma once

#include "Logging/LogVerbosity.h"

// 로그 카테고리다. 로그마다 어느 시스템에서 남겼는지 붙이고, 카테고리마다 출력할 레벨을 따로 정한다.
struct FLogCategory
{
	// 출력할 때 붙이는 이름. SE_DECLARE_LOG_CATEGORY가 문자열 리터럴을 넣으므로 프로그램이 끝날 때까지 유효하다
	const char* name;
	// 이 레벨까지 출력한다. 실행 중에 바꿀 수 있다
	ELogVerbosity verbosity;

	// Fatal 예외는 여기서 다루지 않는다. Fatal은 레벨이 컴파일 시간에 정해지므로 SE_LOG가 이 검사 전에 따로 처리한다
	bool IsEnabled(ELogVerbosity level) const
	{
		return level <= verbosity;
	}
};

// 카테고리를 선언한다. 헤더에 두면 include한 모든 파일이 같은 카테고리 하나를 쓴다(C++17 inline 변수).
// 기본 레벨은 Log라서 Verbose 이하는 카테고리에서 켜야 출력된다.
#define SE_DECLARE_LOG_CATEGORY(CategoryName) inline FLogCategory CategoryName = {#CategoryName, ELogVerbosity::Log}
