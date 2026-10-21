#pragma once

#include "Core/Containers/Array.h"
#include "Input/InputActionKeyMapping.h"
#include "Object.h"

////////////////////////////////////////////////////////////////////////////////
// 키와 액션의 매핑 목록을 소유한다. 입력 평가는 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class UInputMappingContext final : public UObject
{
public:
	/** 빈 액션, 잘못된 키, 유한하지 않은 배율이면 목록을 유지하고 false다. 유효한 항목은 중복을 허용해 끝에 추가한다. */
	[[nodiscard]] bool MapKey(TSharedPtr<const UInputAction> action, EKey key, float scale = 1.0f);

	/** 목록을 읽기 전용으로 빌린다. Context 파괴 후 참조를 사용하지 않으며 등록 후 원소 참조, 반복자는 다시 얻는다. */
	const TArray<FInputActionKeyMapping>& GetMappings() const { return mappings; }

private:
	TArray<FInputActionKeyMapping> mappings;
};
