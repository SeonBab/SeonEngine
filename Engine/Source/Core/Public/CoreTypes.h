#pragma once

// 크기를 명시한 정수 타입이다.
// <cstdint>를 거치지 않고 기본 타입에 직접 연결해서, 실제 타입을 플랫폼 헤더가 아니라 엔진이 정한다.
// (std::int64_t는 플랫폼 헤더가 정한다. Windows에서는 long long, 64비트 Linux에서는 long이다.)
// 지금은 Windows(MSVC)만 지원하므로 플랫폼별 정의 계층 없이 한 곳에 둔다.

using int8 = signed char;
using int16 = signed short;
using int32 = signed int;
using int64 = signed long long;

using uint8 = unsigned char;
using uint16 = unsigned short;
using uint32 = unsigned int;
using uint64 = unsigned long long;

// 표준은 char 계열(1바이트)을 제외하면 최소 크기만 보장하므로 컴파일 시간에 확인한다.
static_assert(sizeof(int8) == 1);
static_assert(sizeof(int16) == 2);
static_assert(sizeof(int32) == 4);
static_assert(sizeof(int64) == 8);

static_assert(sizeof(uint8) == 1);
static_assert(sizeof(uint16) == 2);
static_assert(sizeof(uint32) == 4);
static_assert(sizeof(uint64) == 8);
