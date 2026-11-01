#pragma once

#include "Core/CoreTypes.h"

/** 고정 메시 셰이더의 두 텍스처 연산이다. 숫자는 PS b2와 공유하므로 HLSL 분기와 함께 변경한다. */
enum class ETextureBlendMode : uint32
{
	Lerp = 0,     // 두 RGBA를 비율에 따라 보간한다.
	AddSmooth = 1 // 두 번째 RGB를 마스크로 더하고 첫 번째 알파는 유지한다.
};
