#pragma once

#include "GenericPlatform/GenericPlatformInput.h"
#include "HAL/PreprocessorHelpers.h"

// 빌드 플랫폼의 입력 구현을 선택한다. 선택된 헤더가 FPlatformInput 별칭을 제공한다.
#include SE_COMPILED_PLATFORM_HEADER(PlatformInput.h)
