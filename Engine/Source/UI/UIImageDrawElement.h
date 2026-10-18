#pragma once

#include "Core/Math/Vector2.h"

class UTexture2D;

////////////////////////////////////////////////////////////////////////////////
// 화면에 표시할 이미지 한 개의 그리기 정보다.
// 텍스처를 소유하지 않으므로 사용하는 동안 참조 대상이 살아 있어야 한다.
////////////////////////////////////////////////////////////////////////////////
struct FUIImageDrawElement
{
	const UTexture2D* texture = nullptr;
	FVector2 position{};
	FVector2 size{};
};
