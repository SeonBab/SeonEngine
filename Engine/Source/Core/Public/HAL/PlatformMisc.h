#pragma once

#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PreprocessorHelpers.h"

// 운영체제마다 구현이 다른 작은 기능 모음(FPlatformMisc)의 입구다. Unreal HAL/PlatformMisc.h에 해당한다.
// 빌드하는 플랫폼의 헤더를 include하고, 그 헤더가 FPlatformMisc라는 이름을 플랫폼 struct에 붙인다.
// 함수 목록과 설명은 GenericPlatform/GenericPlatformMisc.h에 있다.
#include SE_COMPILED_PLATFORM_HEADER(PlatformMisc.h)
