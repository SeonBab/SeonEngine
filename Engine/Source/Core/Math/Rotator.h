#pragma once

#include "Core/Math/Quat.h"

////////////////////////////////////////////////////////////////////////////////
// pitch(X)·yaw(Y)·roll(Z)을 도 단위로 보관한다. 기본값은 모두 0이다.
// +X 오른쪽·+Y 위·+Z 앞의 왼손 회전이며 고정 기준 축 Z→X→Y 순서로 해석한다.
// ToQuat은 유한한 각도를 전제로 정규화된 회전을 반환한다.
////////////////////////////////////////////////////////////////////////////////
struct FRotator
{
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;

	[[nodiscard]] FQuat ToQuat() const;
};
