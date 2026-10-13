#pragma once

#if !SE_PLATFORM_WINDOWS
#error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
#endif

#include "GenericPlatform/GenericPlatformInput.h"

////////////////////////////////////////////////////////////////////////////////
// 공통 키 매핑 계약의 Windows 구현 선언이다. Win32 타입을 공개하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FWindowsPlatformInput : public FGenericPlatformInput
{
	/** W / A / S / D의 Windows 키 번호와 공통 이름을 용량 안에서 채운다. 빈 포인터 / 용량0이면0을 반환한다. */
	static uint32 GetKeyMap(uint32* keyCodes, FString* keyNames, uint32 maxMappings);
};

using FPlatformInput = FWindowsPlatformInput;
