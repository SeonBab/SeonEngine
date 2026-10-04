#pragma once

#include "Containers/SeonString.h"
#include "Logging/LogRecord.h"

// 출력 장치들이 공통으로 쓰는 도우미 함수 모음이다. Unreal의 같은 이름 struct와 역할이 같다.
struct FOutputDeviceHelper
{
	/**
	 * 기록을 "[2026.10.05-12.34.56:789]LogWindows: Error: 메시지" 모양의 한 줄로 만든다.
	 * 시각은 UTC로 쓰고, Log 레벨은 레벨 글자를 생략한다(Unreal과 같다). 줄바꿈은 붙이지 않는다.
	 */
	static FString FormatLogLine(const FLogRecord& record);
};
