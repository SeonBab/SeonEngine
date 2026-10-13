#pragma once

#include "Engine/EngineBaseTypes.h"
#include "InputCoreTypes.h"

/** 게임 측에 전달할 키 입력 한 번의 데이터다. 현재 눌림 상태나 처리 결과는 보관하지 않는다. */
struct FInputKeyEventArgs
{
public:
	/** 키와 입력 종류를 값으로 보관한다. 키의 유효성이나 입력 종류를 검사하지 않는다. */
	FInputKeyEventArgs(const FKey& inKey, EInputEvent inEvent)
		: key(inKey),
		  event(inEvent)
	{
	}

	/** 입력이 발생한 키다. 원본 키의 복사본이므로 원본 수명에 의존하지 않는다. */
	FKey key;

	/** 키의 누름·뗌·반복 중 어떤 입력인지 나타낸다. */
	EInputEvent event;
};
