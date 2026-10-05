#pragma once

#if !SE_PLATFORM_WINDOWS
#error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
#endif

#include "Containers/SeonString.h"
#include "GenericPlatform/GenericPlatformProcess.h"

#include <optional>

// FGenericPlatformProcess의 Windows 구현 선언이다. 본문은 Platform/Windows/WindowsPlatformProcess.cpp에 있다.
// <windows.h>를 include하지 않는다. 이 헤더는 FPlatformProcess를 쓰는 모든 파일이 보기 때문이다.
struct FWindowsPlatformProcess : public FGenericPlatformProcess
{
	static std::optional<FString> BaseDir();
	static std::optional<FString> ExecutableName();
};

using FPlatformProcess = FWindowsPlatformProcess;
