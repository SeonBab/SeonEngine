#pragma once

#include "HAL/PreprocessorHelpers.h"

// 플랫폼마다 값이 다른 상수의 입구다. Unreal HAL/Platform.h에 해당한다. 엔진 코드는 이 헤더만 include한다.
// 빌드하는 플랫폼의 헤더(Windows/WindowsPlatform.h 등)가 값을 정의한다. 플랫폼마다 같은 이름의 상수를 반드시 정의한다.
#include SE_COMPILED_PLATFORM_HEADER(Platform.h)
