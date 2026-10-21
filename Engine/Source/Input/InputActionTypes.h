#pragma once

#include "Core/CoreTypes.h"

/** 액션의 단순 활성 상태다. 원시 키의 눌림 상태나 조건 평가 중인 상태와 구분한다. */
enum class EInputActionState : uint8
{
	Inactive,
	Active
};

/** 상태 전이로 발생한 개별 액션 이벤트다. 비트 플래그가 아니며 여러 이벤트는 순서대로 다룬다. */
enum class EInputActionEvent : uint8
{
	None,
	Started,
	Triggered,
	Completed
};

/** 같은 액션에 연결된 여러 매핑값의 결합 방식을 구분한다. */
enum class EInputActionAccumulationBehavior : uint8
{
	/** 절댓값이 가장 큰 입력값을 선택한다. */
	TakeHighestAbsoluteValue,
	/** 여러 입력값을 합산한다. */
	Cumulative
};
