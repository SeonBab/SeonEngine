#pragma once

#include "Core/CoreTypes.h"

/** Windows 가상 키 코드와 구분되는 엔진용 키다. */
enum class EKey : uint8
{
	Invalid,
	W,
	A,
	S,
	D,
	PageUp,
	PageDown,
	Backtick,
	Insert,
	Delete,
	Home,
	End,
	Backspace,
	Count
};

/** 키 입력 사건의 종류다. 누르는 동안 매 프레임 실행하는 조건과는 구분한다. */
enum class EKeyEventType : uint8
{
	Pressed,
	Released,
	Repeat
};

/** 어떤 키에 어떤 입력 사건이 발생했는지 표현한다. 생성만으로 입력을 저장하거나 전달하지 않는다. */
struct FKeyEvent
{
	EKey key = EKey::Invalid;
	EKeyEventType type = EKeyEventType::Pressed;
};
