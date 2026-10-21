#pragma once

#include "Core/Containers/Array.h"
#include "Input/Key.h"

// TODO(seon): 플레이어별 입력이 필요해지면 상태, 이벤트의 소유와 처리 경계를 분리한다.

/** 키의 눌림 상태와 이번 입력 처리 구간의 사건을 보관한다. Windows 메시지나 게임 동작은 처리하지 않는다. */
class FInputState
{
public:
	/** 저장된 눌림 상태를 반환한다. 유효하지 않은 키는 false를 반환한다. */
	bool IsKeyDown(EKey key) const;

	/** 키의 눌림 상태만 저장하며 사건은 추가하지 않는다. 유효하지 않은 키는 무시한다. */
	void SetKeyDown(EKey key, bool isDown);

	/** 유효한 사건을 수신 순서로 추가하고 키 상태도 갱신한다. 잘못된 키, 사건 종류는 무시한다. */
	void AddKeyEvent(FKeyEvent event);

	/** 이번 구간의 사건을 빌려 조회한다. 추가, 초기화 후 원소 참조/반복자를 재사용하지 않는다. */
	const TArray<FKeyEvent>& GetKeyEvents() const;

	/** 사건만 비우고 키의 눌림 상태는 유지한다. 메시지 수집을 시작하기 전에 호출한다. */
	void ResetFrameState();

	/** 모든 키를 해제하고 사건도 비운다. 해제 사건을 새로 만들지는 않는다. */
	void Reset();

private:
	bool keyStates[static_cast<uint8>(EKey::Count)]{};
	TArray<FKeyEvent> keyEvents;
};
