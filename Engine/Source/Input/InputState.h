#pragma once

#include "Input/Key.h"

/** 키의 눌림 상태를 보관한다. Windows 메시지나 게임 동작은 처리하지 않는다. */
class FInputState
{
public:
	/** 저장된 눌림 상태를 반환한다. 유효하지 않은 키는 false를 반환한다. */
	bool IsKeyDown(EKey key) const;

	/** 키의 눌림 상태를 저장한다. 유효하지 않은 키는 무시한다. */
	void SetKeyDown(EKey key, bool isDown);

	/** 모든 키를 눌리지 않은 상태로 초기화한다. */
	void Reset();

private:
	bool keyStates[static_cast<uint8>(EKey::Count)]{};
};
