#pragma once

#include "Core/CoreTypes.h"

/** 텍스처 픽셀의 RGB에 알파가 미리 곱해져 있는지 나타낸다. 블렌드 상태나 그리기 효과와 구분한다. */
enum class ETextureAlphaRepresentation : uint8
{
	// RGB에 알파를 미리 곱하지 않은 표현이다.
	Straight,
	// RGB에 알파를 미리 곱한 표현이다.
	Premultiplied
};
