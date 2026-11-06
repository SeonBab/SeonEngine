#pragma once

#include <cstddef>

/**
 * 플랫폼 키 코드와 분리한 엔진 키 이름이다.
 * A부터 Z, Number0부터 Number9는 순서대로 연속해야 한다. 플랫폼 키 변환이 이 순서에 의존한다.
 */
enum class Key : unsigned char
{
	Unknown = 0,

	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,

	Number0,
	Number1,
	Number2,
	Number3,
	Number4,
	Number5,
	Number6,
	Number7,
	Number8,
	Number9,

	Space,
	Enter,
	Escape,
	Tab,
	// 왼쪽과 오른쪽을 구분하지 않는다.
	Shift,
	Control,
	Alt,

	Up,
	Down,
	Left,
	Right,

	// 키별 배열 크기로만 사용하며 실제 키가 아니다.
	Count
};

constexpr std::size_t KeyCount = static_cast<std::size_t>(Key::Count);

enum class KeyEventType : unsigned char
{
	Pressed,
	Released
};

/** 한 프레임 동안 들어온 키 변화 하나다. */
struct KeyEvent
{
	Key key = Key::Unknown;
	KeyEventType type = KeyEventType::Pressed;
};
