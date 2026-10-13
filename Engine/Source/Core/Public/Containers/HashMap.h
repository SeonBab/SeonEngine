#pragma once

#include "Templates/TypeHash.h"

#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
// 표준 컨테이너를 내부에 보관하고 Unreal식 최소 맵 API를 제공한다.
////////////////////////////////////////////////////////////////////////////////
template<typename KeyType, typename ValueType>
class THashMap
{
public:
	/** 항목을 추가하거나 기존 값을 교체하고 보관한 값의 참조를 반환한다. */
	ValueType& Add(const KeyType& key, const ValueType& value)
	{
		const auto result = entries.insert_or_assign(key, value);

		return result.first->second;
	}

	/** 보관한 값의 비소유 포인터를 반환한다. 항목이 없으면 nullptr다. */
	ValueType* Find(const KeyType& key)
	{
		const auto it = entries.find(key);
		if (it == entries.end())
		{
			return nullptr;
		}

		return &it->second;
	}

	/** 보관한 값의 읽기 전용 비소유 포인터를 반환한다. 항목이 없으면 nullptr다. */
	const ValueType* Find(const KeyType& key) const
	{
		const auto it = entries.find(key);
		if (it == entries.end())
		{
			return nullptr;
		}

		return &it->second;
	}

	/** 모든 항목을 제거하고 버킷 공간은 유지한다. 기존 값의 포인터와 참조는 무효가 된다. */
	void Reset()
	{
		entries.clear();
	}

private:
	/** 키별 GetTypeHash를 호출하며 Core는 구체적인 입력 키 타입을 알지 않는다. */
	std::unordered_map<KeyType, ValueType, TDefaultHash<KeyType>> entries;
};
