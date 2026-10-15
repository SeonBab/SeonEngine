#pragma once

#include "ActorComponent.h"
#include "Containers/Array.h"
#include "Object.h"
#include "Templates/UniquePtr.h"

#include <concepts>

////////////////////////////////////////////////////////////////////////////////
// 컴포넌트를 단독 소유하여 게임 객체 하나를 구성한다.
// 소속이 자신인 컴포넌트를 받아 보관하고 소멸 시 함께 정리한다.
////////////////////////////////////////////////////////////////////////////////
class AActor : public UObject
{
public:
	/** 기본 트랜스폼 컴포넌트 하나를 생성해 소유한다. 추가 중복 정책은 별도다. */
	AActor();

	/** 기반 포인터를 통한 파생 액터 파괴를 지원하고 소유한 컴포넌트를 정리한다. */
	~AActor() override;

	AActor(const AActor&) = delete;
	AActor& operator=(const AActor&) = delete;
	AActor(AActor&&) = delete;
	AActor& operator=(AActor&&) = delete;

	/**
	 * 소속이 자신인 컴포넌트의 소유권을 받는다. 성공 시 입력 포인터는 비워진다.
	 * 빈 포인터·다른 소속·복수 불허인 같은 실제 타입의 중복이면 false이며 입력과 기존 목록을 유지한다.
	 * 복수 허용 정책은 실제 객체에서 조회한다. 서로 다른 파생 타입은 별개로 취급한다.
	 * 메모리 부족은 false 반환으로 복구하지 않는다. 이벤트 호출은 없다.
	 */
	[[nodiscard]] bool AddComponent(TUniquePtr<FActorComponent>& component);

	/**
	 * T로 변환 가능한 첫 컴포넌트를 반환하며 없으면 nullptr다. 검색은 자기 목록에 한정한다.
	 * T는 FActorComponent 자신 또는 public 파생 타입이어야 한다.
	 * 반환 포인터는 비소유이며 컴포넌트 파괴 시 무효다. const 액터에서도 컴포넌트를 수정할 수 있다.
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

	/**
	 * 자기 목록에서 T와 파생 타입의 모든 컴포넌트를 추가 순서대로 조회한다.
	 * T는 FActorComponent 자신 또는 public 파생 타입이어야 한다.
	 * 출력의 기존 내용을 비우고 채우며 확보된 공간은 재사용한다. 일치가 없으면 빈 배열이다.
	 * 포인터는 비소유이며 컴포넌트 파괴 시 무효다. const 액터에서도 조회한 컴포넌트는 수정할 수 있다.
	 */
	template<typename T>
		requires std::derived_from<T, FActorComponent>
	void GetComponents(TArray<T*>& outComponents) const
	{
		outComponents.clear();

		for (const auto& component : components)
		{
			if (T* found = dynamic_cast<T*>(component.get()))
			{
				outComponents.push_back(found);
			}
		}
	}

private:
	TArray<TUniquePtr<FActorComponent>> components{};
};
