#pragma once

#include "InputCoreTypes.h"

////////////////////////////////////////////////////////////////////////////////
// 입력 한 번의 키 식별값과 반복 여부를 전달하는 값 타입이다.
// 눌림 상태를 관리하지 않으며 Unreal의 FKeyEvent를 최소 범위로 준비한다.
////////////////////////////////////////////////////////////////////////////////
struct FKeyEvent
{
public:
	/** 키 값을 복사해 보관한다. 원본 키의 수명에 의존하지 않는다. */
	FKeyEvent(const FKey& inKey, bool bInIsRepeat);

	/** 이벤트의 키 식별값을 복사해 반환한다. */
	FKey GetKey() const;

	/** 이 이벤트가 반복 입력이면 true다. */
	bool IsRepeat() const;

private:
	FKey key;
	bool bIsRepeat;
};
