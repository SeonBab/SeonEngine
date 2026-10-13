#pragma once

#include "CoreTypes.h"

#include <cstddef>

/** 플랫폼 입력 매핑에서 사용하는 숫자 코드의 해시다. 현재는 uint32만 지원한다. */
inline uint32 GetTypeHash(uint32 key)
{
	return key;
}

////////////////////////////////////////////////////////////////////////////////
// std::unordered_map이 엔진의 GetTypeHash를 호출하게 하는 내부 어댑터다.
// TDefaultHash는 자체 타입 이름이며 Unreal의 동명 타입을 구현한 것이 아니다.
// 사용자 정의 키의 GetTypeHash는 인수 의존 탐색(ADL)으로 찾는다.
////////////////////////////////////////////////////////////////////////////////
template<typename KeyType>
struct TDefaultHash
{
	std::size_t operator()(const KeyType& key) const
	{
		return GetTypeHash(key);
	}
};
