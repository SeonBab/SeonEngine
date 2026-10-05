#pragma once

#if !SE_PLATFORM_WINDOWS
#error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
#endif

#include "Containers/SeonString.h"
#include "GenericPlatform/GenericPlatformMisc.h"

// FGenericPlatformMisc의 Windows 구현 선언이다. 본문은 Platform/Windows/WindowsPlatformMisc.cpp에 있다.
// <windows.h>를 include하지 않는다. 이 헤더는 FPlatformMisc를 쓰는 모든 파일이 보기 때문이다.
struct FWindowsPlatformMisc : public FGenericPlatformMisc
{
	static bool IsDebuggerPresent();
	static void LowLevelOutputDebugString(FStringView message);
};

using FPlatformMisc = FWindowsPlatformMisc;
