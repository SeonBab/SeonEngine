#pragma once

#include "Containers/SeonString.h"

// 플랫폼마다 값이 다른 상수를 모은다. 엔진 코드는 이 헤더만 include한다.
// 지금은 Windows만 지원해서 Windows 값을 바로 둔다. 플랫폼이 늘면 플랫폼별 헤더(Windows/WindowsPlatform.h 등)로
// 나누고 이 헤더가 빌드하는 플랫폼의 헤더를 골라 include한다. 쓰는 쪽은 고치지 않는다.

// 텍스트 한 줄의 끝. Windows는 CR+LF다
inline constexpr FStringView LineTerminator = "\r\n";
