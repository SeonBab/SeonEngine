#include "Logging/OutputDeviceDebug.h"

#include "HAL/Platform.h"
#include "HAL/PlatformMisc.h"
#include "Logging/OutputDeviceHelper.h"

void FOutputDeviceDebug::Write(const FLogRecord& record)
{
	// 디버거 출력은 줄바꿈을 붙여 주지 않아서, 붙이지 않으면 다음 로그가 같은 줄에 이어진다
	FString line = FOutputDeviceHelper::FormatLogLine(record);
	line += LineTerminator;
	FPlatformMisc::LowLevelOutputDebugString(line);
}
