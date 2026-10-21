#pragma once

#include "ActorComponent.h"
#include "Core/Containers/Array.h"
#include "Core/Templates/SharedPtr.h"
#include "Input/InputActionTypes.h"
#include "Input/Key.h"

#include <functional>

class FInputActionValue;
class FInputState;
class UInputAction;

// NOTE: 바인딩은 키/액션, 실행 조건, 콜백을 연결하며 캡처 대상의 수명을 보장하지 않는다.

/** 입력 사건과 현재 상태 중 어떤 조건으로 콜백을 실행할지 구분한다. */
enum class EInputTrigger : uint8
{
	Held,
	Pressed
};

/** 프레임 시간을 초 단위로 전달받는 입력 콜백이다. */
using FInputCallback = std::function<void(float)>;

/** 엔진 키, 실행 조건, 콜백을 묶는다. 기본값은 미지원 키, Held, 빈 콜백이다. */
struct FInputKeyBinding
{
	EKey key = EKey::Invalid;
	EInputTrigger trigger = EInputTrigger::Held;
	FInputCallback callback;
};

/** 호출 동안 빌린 액션 값과 초 단위 프레임 시간을 전달받는 콜백이다. 값 참조를 보관하지 않는다. */
using FInputActionCallback = std::function<void(const FInputActionValue&, float)>;

/** 공유 액션 정의, 개별 이벤트, 콜백을 묶는다. 기본값은 빈 액션, None, 빈 콜백이다. */
struct FInputActionBinding
{
	TSharedPtr<const UInputAction> action;
	EInputActionEvent event = EInputActionEvent::None;
	FInputActionCallback callback;
};

// TODO(seon): 프레임별 액션 평가와 전이 결과를 개별 이벤트 전달 경로에 연결한다.

/** 액터의 입력 처리를 담당하는 컴포넌트다. */
class UInputComponent : public UActorComponent
{
public:
	/** 소속 액터를 지정한다. 입력 연결이나 액터 등록은 수행하지 않는다. */
	explicit UInputComponent(AActor& owner);

	/** 유효 키, 실행 조건, 콜백을 등록 순서대로 저장한다. 중복을 허용하며 처리 중이거나 잘못된 키/조건, 빈 콜백이면 false다. */
	[[nodiscard]] bool BindKey(EKey key, EInputTrigger trigger, FInputCallback callback);

	/** 유효 액션, Started/Triggered/Completed, 콜백을 등록 순서대로 저장한다. 중복은 허용하고 처리 중 등록, 빈 액션/콜백, 잘못된 이벤트는 false다. */
	[[nodiscard]] bool BindAction(TSharedPtr<const UInputAction> action, EInputActionEvent event, FInputActionCallback callback);

	/**
	 * 같은 액션 객체와 이벤트의 콜백을 등록 순서대로 호출한다. None, 잘못된 이벤트, 키/액션 처리 중 중첩 호출은 무시한다.
	 * 값은 액션 형식의 유효한 평가 결과여야 하며 호출 동안만 빌린다. 처리 중 값과 바인딩을 변경하지 않는다.
	 * 콜백은 정상 반환해야 하며 이 컴포넌트와 소유 액터를 즉시 파괴하면 안 된다. 키/액션 처리 중 두 종류의 바인딩 등록은 거부한다.
	 * 계산, 상태 갱신, 전이 해석, 소비는 수행하지 않는다. 동일 이벤트를 다시 전달하면 다시 호출하며 시간값은 그대로 전달한다.
	 */
	void DispatchActionEvent(const UInputAction& action, EInputActionEvent event, const FInputActionValue& value, float deltaSeconds);

	/**
	 * Pressed는 사건 순서, 같은 사건 안에서는 등록 순서로, 그 뒤 Held는 눌린 키의 등록 순서로 실행한다.
	 * 사건은 소비하지 않으며 호출할 때마다 같은 입력을 다시 처리한다. 초 단위 시간값을 그대로 전달하고 중첩 호출은 무시한다.
	 * 입력 상태는 처리 중 변경하면 안 된다. 키/액션 중첩 호출은 무시하고 처리 중 BindKey/BindAction은 거부한다. 콜백은 정상 반환해야 하며 이 컴포넌트와 소유 액터를 즉시 파괴하면 안 된다.
	 */
	void ProcessInput(const FInputState& input, float deltaSeconds);

private:
	TArray<FInputKeyBinding> keyBindings;
	TArray<FInputActionBinding> actionBindings;
	bool bProcessingInput = false;
};
