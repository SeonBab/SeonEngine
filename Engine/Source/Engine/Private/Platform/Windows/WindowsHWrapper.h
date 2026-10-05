#pragma once

// Windows.h는 이 래퍼로만 include한다.
// Public 헤더에서 include하면 매크로와 Windows 타입이 게임 코드까지 퍼지므로 include하지 않는다.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// 엔진 함수 이름과 충돌하는 매크로
#undef CreateWindow
