#pragma once

#include "GenericPlatform/GenericPlatformProcess.h"
#include "HAL/PreprocessorHelpers.h"

// 실행 중인 프로그램(프로세스)에 대해 운영체제에 묻는 기능 모음(FPlatformProcess)의 입구다. Unreal HAL/PlatformProcess.h에 해당한다.
// 빌드하는 플랫폼의 헤더를 include하고, 그 헤더가 FPlatformProcess라는 이름을 플랫폼 struct에 붙인다.
// 함수 목록과 설명은 GenericPlatform/GenericPlatformProcess.h에 있다.
#include SE_COMPILED_PLATFORM_HEADER(PlatformProcess.h)
