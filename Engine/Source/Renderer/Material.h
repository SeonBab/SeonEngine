#pragma once

#include "Core/Math/Vector3.h"

#include <memory>

class Texture;

/** 표면 텍스처와 텍스처 값에 곱할 계수를 보관한다. */
struct Material
{
	// 색상 텍스처다. nullptr이면 미설정 상태다.
	std::shared_ptr<const Texture> baseColorTexture;

	// 색상 텍스처에 성분별로 곱할 선형 RGB 계수다.
	// 각 성분은 0부터 1까지이며, 흰색이면 텍스처 색상을 유지한다.
	Vector3 baseColor = { 1.0f, 1.0f, 1.0f };

	// ORM의 G에 곱할 거칠기 계수다. 범위는 0부터 1까지다.
	// 1이면 텍스처 값을 유지하고, 0이면 최종 거칠기가 0이 된다.
	float roughness = 1.0f;

	// ORM의 B에 곱할 금속성 계수다. 범위는 0부터 1까지다.
	// 1이면 텍스처 값을 유지하고, 0이면 최종 금속성이 0이 된다.
	float metallic = 1.0f;
};
