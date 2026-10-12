#pragma once

#if !SE_PLATFORM_WINDOWS
#error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
#endif

#include "Platform/GenericPlatform/GenericApplication.h"

////////////////////////////////////////////////////////////////////////////////
// Windows 애플리케이션의 메시지 처리 주체다. Win32 타입과 호출은 구현 파일에 둔다.
////////////////////////////////////////////////////////////////////////////////
class FWindowsApplication final : public FGenericApplication
{
public:
	FWindowsApplication() = default;
	~FWindowsApplication() override = default;

	void PumpMessages(float TimeDelta) override;
};
