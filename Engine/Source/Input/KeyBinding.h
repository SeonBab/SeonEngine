#pragma once

#include "Input/Key.h"

#include <functional>

/** 바인딩 실행 조건이다. */
enum class KeyTrigger : unsigned char
{
	// 누름 이벤트 하나당 한 번
	Pressed,
	// 뗌 이벤트 하나당 한 번
	Released,
	// 전달 시점에 누르는 중이면 프레임당 한 번
	Held
};

/** 바인딩에 연결하는 함수다. deltaTime는 초 단위이며 Pressed와 Released에서도 같은 값을 받는다. */
using KeyAction = std::function<void(float deltaTime)>;

/** 키와 실행 조건에 연결한 함수 하나다. */
struct KeyBinding
{
	Key key = Key::Unknown;
	KeyTrigger trigger = KeyTrigger::Pressed;
	KeyAction action;
};
