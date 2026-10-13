#pragma once

#include "CoreTypes.h"

/** 게임 측에 전달할 키 입력의 종류다. 키의 현재 눌림 상태나 이벤트 처리 결과와는 구분한다. */
enum class EInputEvent : uint8
{
	/** 키를 처음 눌렀을 때의 입력이다. */
	IE_Pressed = 0,

	/** 키를 뗐을 때의 입력이다. */
	IE_Released = 1,

	/** 키를 누르고 있는 동안 발생한 반복 입력이다. */
	IE_Repeat = 2
};
