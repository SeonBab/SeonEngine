#pragma once

#include "Core/Math/Vector3.h"

/** Scene에 전달할 균일 환경광 값. 객체나 GPU 자원을 소유하지 않는다. */
struct FSkyLightData
{
	// 각 성분이 유한한 0 이상인 선형 RGB이며 알파는 없다.
	FVector3 lightColor{1.0f, 1.0f, 1.0f};

	// 유한한 0 이상의 밝기 배율. 0은 기여 없음이며 물리 단위는 아니다.
	float intensity = 1.0f;
};
