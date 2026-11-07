#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"

/** 한 시점으로 그리기 위한 카메라 정보의 값 사본이다. 행 벡터 규칙으로 뷰 행렬 다음 투영 행렬을 곱한다. */
struct RenderView
{
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	Vector3 cameraPosition;
};
