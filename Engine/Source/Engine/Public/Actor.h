#pragma once

#include "ActorComponent.h"
#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "Templates/UniquePtr.h"

#include <concepts>
#include <typeinfo>
#include <utility>

class FWorld;

////////////////////////////////////////////////////////////////////////////////
// 월드에 속하는 게임 객체의 기반이다. 기능은 컴포넌트를 소유하여 구성한다.
// 생성 시 트랜스폼 하나를 소유한다. 시스템 씬 연결은 아직 없다.
////////////////////////////////////////////////////////////////////////////////
class FActor
{
public:
	/** 소속 월드를 비소유 연결하고 필수 트랜스폼 하나를 소유한다. 월드는 액터보다 오래 살아 있어야 한다. */
	explicit FActor(FWorld& inWorld);

	/** 기반 포인터로 파괴해도 파생 소멸자를 실행하고 자식·컴포넌트를 정리한다. 시스템 씬 등록 해제는 아직 없다. */
	virtual ~FActor();

	FActor(const FActor&) = delete;
	FActor& operator=(const FActor&) = delete;
	FActor(FActor&&) = delete;
	FActor& operator=(FActor&&) = delete;

	/** 생성 시 지정한 소속 월드를 반환한다. 월드의 수명을 연장하지 않는다. */
	FWorld& GetWorld() const;

	/**
	 * 소속 월드에 자신과 자손의 제거 처리를 요청한다. true는 제거 처리 시작 또는 이미 제거 중이다.
	 * 메모리의 즉시 파괴를 뜻하지 않는다. 월드의 제거 처리에 위임한다.
	 */
	[[nodiscard]] bool Destroy();

	/** 비소유 부모를 반환한다. 부모가 없는 월드 루트이면 nullptr다. */
	[[nodiscard]] FActor* GetParent() const;

	/** 직접 연결된 자식 수를 반환한다. 손자 등 후손은 포함하지 않는다. */
	[[nodiscard]] int32 GetChildCount() const;

	/**
	 * 직접 연결된 자식을 반환한다. 음수 / 범위 밖 인덱스이면 nullptr다.
	 * 반환 포인터는 비소유이며 자식 파괴 시 무효다. const 액터에서도 자식을 수정할 수 있다.
	 * 자식 연결 / 분리 후 목록의 인덱스는 달라질 수 있다.
	 */
	[[nodiscard]] FActor* GetChild(int32 index) const;

	/** 제거 처리가 시작됐는지 반환한다. 객체 메모리의 파괴 여부를 검사하는 함수는 아니다. */
	[[nodiscard]] bool IsActorBeingDestroyed() const;

	/**
	 * 같은 월드의 살아 있는 등록 액터로 부모를 변경한다. nullptr는 루트 분리다.
	 * 유한 값 / 단위 회전이 전제다. true는 변경 완료 또는 같은 부모, false는 기존 상태 유지다.
	 * 자신 또는 새 부모가 제거 중이면 같은 부모 요청도 false다. 재합성 오차로 거부하지 않는다.
	 * Unreal의 AttachToActor / AttachToComponent와 달리 액터 계층과 자식 단독 소유권을 함께 이전한다.
	 * 검사와 새 로컬 계산을 변경 전에 끝내므로 false이면 기존 부모 / 소유권 / 변환을 유지한다.
	 * Unreal 부착에는 기존 연결을 분리하거나 새 부모를 반영한 뒤 false를 반환하는 특수 경로가 있다.
	 * 변환 계산은 Unreal 방식을 따르지만 부착 순서 / 소유 구조 / 실패 상태까지 동일하지는 않다.
	 */
	[[nodiscard]] bool SetParent(FActor* newParent, ETransformRule rule);

	/**
	 * 미리 생성·설정한 컴포넌트의 소유권을 액터로 이전한다. T는 FActorComponent 자신 또는 public 파생 타입이다.
	 * true이면 component는 비워지고 액터가 소유한다. false이면 component의 소유권과 기존 목록을 유지한다.
	 * 빈 포인터 / 다른 소속 / 제거 중 액터 / 복수 불허인 같은 실제 타입의 중복이면 false다.
	 * 기반 타입 포인터도 허용하며 정책과 타입은 실제 객체에서 조회한다. 트랜스폼도 같은 규칙을 적용한다.
	 * 객체 주소는 이전 후에도 유지한다. 시스템 등록 / 이벤트 호출은 아직 없다.
	 */
	template<typename T>
		requires std::derived_from<T, FActorComponent>
	[[nodiscard]] bool AddComponent(TUniquePtr<T>& component)
	{
		if (!component || &component->GetOwner() != this || IsActorBeingDestroyed()) { return false; }

		if (!component->AllowsMultipleComponents())
		{
			const std::type_info& componentType = typeid(*component);
			for (const auto& existingComponent : components)
			{
				if (typeid(*existingComponent) == componentType) { return false; }
			}
		}

		// 소유권 이전 전에 공간을 확보해 목록 재할당과 객체 이전을 분리한다.
		if (components.size() == components.capacity())
		{
			components.reserve(components.empty() ? 1 : components.size() * 2);
		}
		components.push_back(std::move(component));

		return true;
	}

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

	/**
	 * 자기 액터에서 T와 파생 타입의 모든 컴포넌트를 목록 순서로 조회한다. 자식 액터는 검색하지 않는다.
	 * 기존 출력 내용을 비우고 채우며, 일치가 없으면 빈 배열이다. 출력 배열의 확보된 공간은 재사용한다.
	 * 포인터는 비소유이며 컴포넌트 파괴 시 무효다. const 액터에서도 조회한 컴포넌트는 수정할 수 있다.
	 * 출력 배열을 수정해도 액터의 소유 목록은 변경되지 않는다.
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
	friend class FWorld;

	bool IsOwnedByWorld() const;

	FWorld* const world;
	FActor* parent = nullptr;
	bool bActorIsBeingDestroyed = false;
	TArray<TUniquePtr<FActorComponent>> components;
	TArray<TUniquePtr<FActor>> children;
};
