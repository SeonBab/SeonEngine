#pragma once

#include "Core/Templates/SharedPtr.h"
#include "Input/InputAction.h"
#include "Input/InputActionEvaluator.h"

#include <utility>

////////////////////////////////////////////////////////////////////////////////
// 액션 하나의 실행 중 값, 상태, 전이 결과를 보관한다. 키 평가나 이벤트 전달은 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FInputActionInstance final
{
public:
	/** 유효한 값 형식의 비어 있지 않은 액션을 받아, 해당 형식의 0값, Inactive, None으로 시작한다. */
	explicit FInputActionInstance(TSharedPtr<const UInputAction> sourceAction)
		: action(std::move(sourceAction))
	{
		if (action->GetValueType() == EInputActionValueType::Axis1D)
		{
			value = FInputActionValue(0.0f);
		}
	}

	/**
	 * 형식이 다르거나 비유한 값이면 기존 데이터를 유지하고 false다.
	 * 정상 값은 정확한 0 여부로 상태를 판정해 값, 상태, 전이 결과를 함께 갱신한다. 한 호출이 한 평가다.
	 */
	[[nodiscard]] bool UpdateValue(const FInputActionValue& newValue);

	/** 공유 소유한 액션을 빌린다. 반환 참조는 이 인스턴스의 액션 참조가 유지되는 동안 유효하다. */
	const UInputAction& GetSourceAction() const { return *action; }

	/** 현재 값을 빌린다. 반환 참조는 인스턴스가 살아 있고 이동되지 않는 동안 유효하며 갱신 후 내용은 달라질 수 있다. */
	const FInputActionValue& GetValue() const { return value; }

	EInputActionState GetState() const { return state; }
	EInputActionTransitionResult GetTransitionResult() const { return transitionResult; }

private:
	TSharedPtr<const UInputAction> action;
	FInputActionValue value;
	EInputActionState state = EInputActionState::Inactive;
	EInputActionTransitionResult transitionResult = EInputActionTransitionResult::None;
};
