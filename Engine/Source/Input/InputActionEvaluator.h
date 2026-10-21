#pragma once

#include "Input/InputActionTypes.h"
#include "Input/InputActionValue.h"

#include <optional>

class FInputState;
class UInputAction;
class UInputMappingContext;

/**
 * 같은 액션 객체에 연결된 눌린 키의 배율을 액션의 결합 규칙으로 계산한다. 입력이 없으면 액션 형식의 false 또는 0을 반환한다.
 * Cumulative는 합산, TakeHighestAbsoluteValue는 최대 절댓값 선택이며 동률은 나중 매핑을 선택한다.
 * Boolean은 결합값이 정확히 0이 아닌지 판정해 0/1로 저장하고 Axis1D는 수치를 유지한다.
 * 지원하지 않는 형식/규칙, 유한한 float 범위 밖 결과는 nullopt다. clamp나 시간 곱셈은 하지 않는다.
 * 입력, 사건, Context를 변경하지 않는다. 호출 중 입력 상태나 매핑 목록을 변경하면 안 된다.
 */
[[nodiscard]] std::optional<FInputActionValue> EvaluateActionValue(const UInputAction& action, const UInputMappingContext& context, const FInputState& input);

/** 전이 평가의 내부 결과다. StartedAndTriggered는 전달 시 Started 다음 Triggered로 해석한다. */
enum class EInputActionTransitionResult : uint8
{
	None,
	StartedAndTriggered,
	Triggered,
	Completed
};

/** 유효한 액션 상태 두 개로 전이 결과만 계산한다. 상태 저장이나 이벤트 전달은 하지 않는다. */
[[nodiscard]] EInputActionTransitionResult EvaluateActionTransition(EInputActionState previousState, EInputActionState currentState);
