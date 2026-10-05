#pragma once

#include "Containers/SeonString.h"
#include "Logging/LogCategory.h"
#include "Logging/LogVerbosity.h"

#include <chrono>

// 로그를 남긴 시각이다. 출력할 때 UTC로 표시한다
using FLogTime = std::chrono::system_clock::time_point;

// 로그 한 건의 정보다. 전달기(FOutputDeviceRedirector)가 한 번 만들어 모든 출력 장치에 넘긴다.
// 메시지는 꾸미지 않은 원래 문자열이고, 시각 / 카테고리 / 레벨을 붙인 한 줄이 필요한 장치는 FOutputDeviceHelper::FormatLogLine으로 만든다.
struct FLogRecord
{
	const FLogCategory& category;
	ELogVerbosity verbosity;
	// 출력 장치의 Write 호출 중에만 유효하다. 기록을 보관하는 장치는 FString으로 복사한다
	FStringView message;
	// 전달기가 한 번 재서 넣으므로 모든 출력 장치가 같은 시각을 쓴다
	FLogTime time;
};
