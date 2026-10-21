#pragma once

#include "Core/Templates/SharedPtr.h"
#include "Input/Key.h"

class UInputAction;

////////////////////////////////////////////////////////////////////////////////
// 키와 액션의 대응 및 입력 배율을 보관한다. 등록 검사나 입력 평가는 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FInputActionKeyMapping
{
	TSharedPtr<const UInputAction> action;
	EKey key = EKey::Invalid;
	float scale = 1.0f;
};
