#pragma once

#include "Containers/SeonString.h"
#include "CoreTypes.h"

////////////////////////////////////////////////////////////////////////////////
// 플랫폼별 키 번호와 공통 키 이름의 매핑을 제공하는 계약이다.
// 현재는 선언만 준비하며 호출 연결과 구현은 후속 단계에서 추가한다.
////////////////////////////////////////////////////////////////////////////////
struct FGenericPlatformInput
{
	/** 호출자가 준비한 두 배열에 최대 maxMappings개의 쌍을 채우고 실제 개수를 반환한다. 두 배열은 각각 그 개수만큼 유효한 요소를 가져야 하며 소유권은 이전하지 않는다. */
	static uint32 GetKeyMap(uint32* keyCodes, FString* keyNames, uint32 maxMappings);
};
