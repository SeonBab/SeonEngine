#pragma once

/**
 * 도 단위 오일러 각이다. pitch는 X축, yaw는 Y축, roll은 Z축 회전이며 Z, X, Y 축 순서로 적용한다.
 * 양수 pitch는 +Z를 -Y 쪽으로, 양수 yaw는 +Z를 +X 쪽으로, 양수 roll은 +Y를 -X 쪽으로 돌린다.
 * 값의 범위는 보정하지 않는다.
 */
struct EulerAngles
{
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
};
