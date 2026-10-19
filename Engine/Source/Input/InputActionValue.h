#pragma once

#include "Core/CoreTypes.h"

/** 액션이 전달할 값의 종류다. 입력 사건이나 발동 조건과 구분한다. */
enum class EInputActionValueType : uint8
{
	Boolean,
	Axis1D
};

////////////////////////////////////////////////////////////////////////////////
// 액션 값과 형식을 함께 보관한다. 키 매핑이나 발동 조건은 평가하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FInputActionValue final
{
public:
	/** Boolean 형식의 false 값으로 생성한다. */
	FInputActionValue() = default;

	/** Boolean 형식으로 생성하고 false/true를 0/1로 보관한다. */
	explicit FInputActionValue(bool value)
		: valueType(EInputActionValueType::Boolean),
		  value(value ? 1.0f : 0.0f)
	{
	}

	/** Axis1D 형식으로 생성하고 전달한 값을 그대로 보관한다. */
	explicit FInputActionValue(float value)
		: valueType(EInputActionValueType::Axis1D),
		  value(value)
	{
	}

	EInputActionValueType GetValueType() const { return valueType; }

	/** 형식과 관계없이 저장값이 정확히 0이 아니면 true다. 허용 오차는 적용하지 않는다. */
	bool GetBoolean() const { return value != 0.0f; }

	/** 저장값을 반환한다. Boolean 형식이면 0 또는 1이다. */
	float GetAxis1D() const { return value; }

private:
	EInputActionValueType valueType = EInputActionValueType::Boolean;
	float value = 0.0f;
};
