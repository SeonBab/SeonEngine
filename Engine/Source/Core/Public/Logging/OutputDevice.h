#pragma once

#include "Logging/LogRecord.h"

// 로그 출력 장치다(VS 출력 창, 파일 등). Unreal FOutputDevice에 해당한다. Core는 이 인터페이스만 알고, 구현은 Platform 등 상위 코드가 만들어 FOutputDeviceRedirector에 등록한다.
class IOutputDevice
{
public:
	virtual ~IOutputDevice() = default;

	// 로그 한 건을 출력한다. record.message는 이 호출 중에만 유효하다
	virtual void Write(const FLogRecord& record) = 0;
};
