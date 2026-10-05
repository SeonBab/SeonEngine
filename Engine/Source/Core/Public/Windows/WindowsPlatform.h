#pragma once

#if !SE_PLATFORM_WINDOWS
#error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
#endif

#include "Containers/SeonString.h"

// Windows에서 값이 정해지는 상수다. 쓰는 쪽은 이 헤더가 아니라 HAL/Platform.h를 include한다.

// 텍스트 한 줄의 끝. Windows는 CR+LF다
inline constexpr FStringView LineTerminator = "\r\n";
