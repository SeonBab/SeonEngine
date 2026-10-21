#pragma once

#include "Input/InputActionTypes.h"
#include "Input/InputActionValue.h"
#include "Object.h"

////////////////////////////////////////////////////////////////////////////////
// 입력 행동의 정의, 값 형식, 결합 규칙을 표현한다. 현재 입력값이나 게임 콜백은 보관하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class UInputAction final : public UObject
{
public:
	/** 유효한 값 형식과 결합 규칙을 지정해 액션을 정의한다. 키 매핑이나 입력 처리는 수행하지 않는다. */
	explicit UInputAction(EInputActionValueType actionValueType, EInputActionAccumulationBehavior actionAccumulationBehavior = EInputActionAccumulationBehavior::TakeHighestAbsoluteValue)
		: valueType(actionValueType),
		  accumulationBehavior(actionAccumulationBehavior)
	{
	}

	EInputActionValueType GetValueType() const { return valueType; }
	EInputActionAccumulationBehavior GetAccumulationBehavior() const { return accumulationBehavior; }

private:
	EInputActionValueType valueType = EInputActionValueType::Boolean;
	EInputActionAccumulationBehavior accumulationBehavior = EInputActionAccumulationBehavior::TakeHighestAbsoluteValue;
};
