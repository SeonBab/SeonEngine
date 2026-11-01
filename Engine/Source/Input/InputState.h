#pragma once

#include "Core/Containers/Array.h"
#include "Core/Math/Vector2.h"
#include "Input/Key.h"

// TODO(seon): 플레이어별 입력이 필요해지면 상태, 이벤트의 소유와 처리 경계를 분리한다.

/** 키·버튼의 눌림 상태와 이번 입력 처리 구간의 사건·상대 마우스 이동량·휠 회전량을 보관한다. Windows 메시지나 게임 동작은 처리하지 않는다. */
class FInputState
{
#pragma region KeyState

public:
	/** 저장된 눌림 상태를 반환한다. 유효하지 않은 키는 false를 반환한다. */
	bool IsKeyDown(const FKey& key) const;

	/** 키의 눌림 상태만 저장하며 사건은 추가하지 않는다. 유효하지 않은 키는 무시한다. */
	void SetKeyDown(const FKey& key, bool isDown);

private:
	TArray<FKey> pressedKeys; // 눌린 유효 키만 중복 없이 보관한다. 해제되면 목록에서 제거한다.

#pragma endregion

#pragma region KeyEvents

public:
	/** 유효한 사건을 수신 순서로 추가하고 키 상태도 갱신한다. 잘못된 키, 사건 종류는 무시한다. */
	void AddKeyEvent(FKeyEvent event);

	/** 이번 구간의 사건을 빌려 조회한다. 추가, 초기화 후 원소 참조/반복자를 재사용하지 않는다. */
	const TArray<FKeyEvent>& GetKeyEvents() const;

	/** 사건과 상대 마우스 이동량·휠 회전량을 비우고 키·버튼의 눌림 상태는 유지한다. 메시지 수집을 시작하기 전에 호출한다. */
	void ResetFrameState();

private:
	TArray<FKeyEvent> keyEvents;

#pragma endregion

#pragma region Mouse

public:
	/** 상대 이동량을 누적한다. +X는 오른쪽, +Y는 아래이며 화면 픽셀이 아닌 Raw Input 상대 이동 단위다. 입력과 누적 결과는 유한한 값을 전제로 하며 검사·보정하지 않는다. */
	void AddMouseDelta(FVector2 delta);

	/** 이번 입력 처리 구간의 상대 이동 누적값을 사본으로 반환하며 비우지 않는다. */
	FVector2 GetMouseDelta() const;

	/** 세로 휠 회전량을 누적한다. 한 눈금은 1이고 양수는 사용자에게서 먼 방향이며 소수 눈금도 허용한다. 입력과 누적 결과는 유한한 값을 전제로 하며 검사·보정하지 않는다. */
	void AddMouseWheelDelta(float delta);

	/** 이번 입력 처리 구간의 세로 휠 누적값을 반환하며 비우지 않는다. */
	float GetMouseWheelDelta() const;

private:
	FVector2 mouseDelta{}; // ResetFrameState 또는 Reset까지 누적하며 다음 수집 구간으로 넘기지 않는다.
	float mouseWheelDelta = 0.0f;

#pragma endregion

#pragma region Reset

public:
	/** 모든 키·버튼을 해제하고 사건과 상대 마우스 이동량·휠 회전량도 비운다. 해제 사건을 새로 만들지는 않는다. */
	void Reset();

#pragma endregion
};
