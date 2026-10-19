#pragma once

#include "ActorComponent.h"
#include "Core/Containers/Array.h"
#include "Input/Key.h"

#include <functional>

class FInputState;

// NOTE: 바인딩은 키·실행 조건·콜백을 연결하며 캡처 대상의 수명을 보장하지 않는다.
// 행동 매핑·키 재설정·입력 우선순위가 필요해지면 구조를 재검토한다.

/** 입력 사건과 현재 상태 중 어떤 조건으로 콜백을 실행할지 구분한다. */
enum class EInputTrigger : uint8
{
	Held,
	Pressed
};

// TODO(seon): 연속 입력의 축 매핑이 필요해지면 유지 상태 실행과 키 사건 바인딩을 분리한다.

/** 프레임 시간을 초 단위로 전달받는 입력 콜백이다. */
using FInputCallback = std::function<void(float)>;

/** 엔진 키·실행 조건·콜백을 묶는다. 기본값은 미지원 키·Held·빈 콜백이다. */
struct FInputKeyBinding
{
	EKey key = EKey::Invalid;
	EInputTrigger trigger = EInputTrigger::Held;
	FInputCallback callback;
};

// TODO(seon): 행동 매핑·재바인딩 요구가 생기면 Unreal 기본 입력 구조를 참고해 바인딩 책임을 확장한다.

/** 액터의 입력 처리를 담당하는 컴포넌트다. */
class UInputComponent : public UActorComponent
{
public:
	/** 소속 액터를 지정한다. 입력 연결이나 액터 등록은 수행하지 않는다. */
	explicit UInputComponent(AActor& owner);

	/** 유효 키·실행 조건·콜백을 등록 순서대로 저장한다. 중복을 허용하며 처리 중이거나 잘못된 키/조건·빈 콜백이면 false다. */
	[[nodiscard]] bool BindKey(EKey key, EInputTrigger trigger, FInputCallback callback);

	/**
	 * Pressed는 사건 순서·같은 사건 안에서는 등록 순서로, 그 뒤 Held는 눌린 키의 등록 순서로 실행한다.
	 * 사건은 소비하지 않으며 호출할 때마다 같은 입력을 다시 처리한다. 초 단위 시간값을 그대로 전달하고 중첩 호출은 무시한다.
	 * 입력 상태는 처리 중 변경하면 안 된다. 처리 중 BindKey는 거부한다. 콜백은 정상 반환해야 하며 이 컴포넌트와 소유 액터를 즉시 파괴하면 안 된다.
	 */
	void ProcessInput(const FInputState& input, float deltaSeconds);

private:
	TArray<FInputKeyBinding> keyBindings;
	bool bProcessingInput = false;
};
