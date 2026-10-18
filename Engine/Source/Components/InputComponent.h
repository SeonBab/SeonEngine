#pragma once

#include "ActorComponent.h"
#include "Core/Containers/Array.h"
#include "Input/Key.h"

#include <functional>

class FInputState;

// NOTE: 현재 바인딩 타입은 키와 콜백의 직접 대응만 표현하며 캡처 대상의 수명을 보장하지 않는다.
// 행동 매핑·키 재설정·입력 우선순위가 필요해지면 구조를 재검토한다.
// 근거: ProjectSetupDecisions의 "최소 키·콜백 바인딩 타입 — 2026-10-18".

/** 프레임 시간을 초 단위로 전달받는 입력 콜백이다. */
using FInputCallback = std::function<void(float)>;

/** 엔진 키와 실행할 콜백을 묶는다. 기본값은 미지원 키와 빈 콜백이다. */
struct FInputKeyBinding
{
	EKey key = EKey::Invalid;
	FInputCallback callback;
};

/** 액터의 입력 처리를 담당하는 컴포넌트다. */
class UInputComponent : public UActorComponent
{
public:
	/** 소속 액터를 지정한다. 입력 연결이나 액터 등록은 수행하지 않는다. */
	explicit UInputComponent(AActor& owner);

	/** 유효 키와 콜백을 등록 순서대로 저장한다. 중복 키를 허용하며 처리 중이거나 잘못된 키·빈 콜백이면 false다. */
	[[nodiscard]] bool BindKey(EKey key, FInputCallback callback);

	/**
	 * 눌린 키의 콜백을 등록 순서대로 실행하고 시간값을 그대로 전달한다. 중첩 호출은 무시한다.
	 * 처리 중 BindKey는 거부한다. 콜백은 정상 반환해야 하며 이 컴포넌트와 소유 액터를 즉시 파괴하면 안 된다.
	 */
	void ProcessInput(const FInputState& input, float deltaSeconds);

private:
	TArray<FInputKeyBinding> keyBindings;
	bool bProcessingInput = false;
};
