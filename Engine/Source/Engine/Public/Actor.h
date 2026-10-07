#pragma once

#include "ActorComponent.h"
#include "Containers/Array.h"
#include "Templates/UniquePtr.h"

#include <concepts>

////////////////////////////////////////////////////////////////////////////////
// 월드에 속하는 게임 객체의 기반이다. 기능은 컴포넌트를 소유하여 구성한다.
// 생성 시 트랜스폼 하나를 소유한다. 시스템 씬 연결은 아직 없다.
////////////////////////////////////////////////////////////////////////////////
class FActor
{
public:
	/** 소속 액터가 자신인 트랜스폼 컴포넌트를 하나 만들어 소유한다. */
	FActor();

	/** 소유한 컴포넌트를 파괴한다. 시스템 씬 등록 해제는 아직 연결하지 않았다. */
	~FActor();

	FActor(const FActor&) = delete;
	FActor& operator=(const FActor&) = delete;
	FActor(FActor&&) = delete;
	FActor& operator=(FActor&&) = delete;

	/**
	 * T로 변환 가능한 첫 컴포넌트를 반환하며, 없으면 nullptr를 반환한다.
	 * T는 FActorComponent 자신 또는 public 파생 타입이어야 한다.
	 * const 액터에서도 컴포넌트를 수정할 수 있다. 반환 포인터는 비소유이며 컴포넌트 파괴 시 무효다.
	 */
	template<typename T>
		requires std::derived_from<T, FActorComponent>
	[[nodiscard]] T* GetComponent() const
	{
		for (const auto& component : components)
		{
			if (T* found = dynamic_cast<T*>(component.get()))
			{
				return found;
			}
		}
		return nullptr;
	}

private:
	TArray<TUniquePtr<FActorComponent>> components;
};
