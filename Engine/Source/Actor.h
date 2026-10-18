#pragma once

#include "Components/ActorComponent.h"
#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Templates/UniquePtr.h"
#include "Object.h"

#include <concepts>

class UWorld;

/** 부모 변경 시 유지할 변환 공간을 지정한다. KeepWorld의 표현 한계는 FTransform 상대 변환을 따른다. */
enum class EActorTransformRule : uint8
{
	KeepLocal,
	KeepWorld
};

/** 액터 종료 처리를 호출하는 이유다. 명시적 제거와 실행 종료를 구분한다. */
enum class EEndPlayReason : uint8
{
	Destroyed,
	Quit
};

////////////////////////////////////////////////////////////////////////////////
// 컴포넌트를 단독 소유하여 게임 객체 하나를 구성한다.
// 소속이 자신인 컴포넌트를 받아 보관하고 소멸 시 함께 정리한다.
////////////////////////////////////////////////////////////////////////////////
class AActor : public UObject
{
public:
	/** 비소유 월드를 연결하고 기본 트랜스폼 하나를 소유한다. 자동 등록은 하지 않으며 월드는 액터보다 오래 살아야 한다. */
	explicit AActor(UWorld& inWorld);

	/** 기반 포인터를 통한 파생 액터 파괴를 지원하고 소유한 컴포넌트를 정리한다. 자식 관계는 비소유다. */
	~AActor() override;

	AActor(const AActor&) = delete;
	AActor& operator=(const AActor&) = delete;
	AActor(AActor&&) = delete;
	AActor& operator=(AActor&&) = delete;

	/** World의 Tick에서 초 단위 프레임 경과 시간을 받는 재정의 지점이다. 기본 구현은 비어 있다. */
	virtual void Tick(float deltaSeconds);

	/** 기반 BeginPlay에서 시작 완료 상태로 전환됐는지 반환한다. */
	[[nodiscard]] bool HasActorBegunPlay() const;

	/** 시작 진입 뒤 기반 BeginPlay가 아직 완료 상태로 바꾸지 않았는지 반환한다. 기반 호출을 생략하면 true가 유지된다. */
	[[nodiscard]] bool IsActorBeginningPlay() const;

	/** 생성 시 지정한 월드를 반환한다. 월드의 수명을 연장하거나 소유 목록 등록 여부를 검사하지 않는다. */
	[[nodiscard]] UWorld& GetWorld() const;

	/** 비소유 부모를 반환한다. 부모가 없으면 nullptr다. */
	[[nodiscard]] AActor* GetParent() const;

	/** 직접 연결된 자식 수를 반환한다. 손자 등 후손은 포함하지 않는다. */
	[[nodiscard]] int32 GetChildCount() const;

	/**
	 * 직접 연결된 자식을 반환하며 음수·범위 밖 인덱스는 nullptr다.
	 * 반환 포인터는 비소유이며 자식 파괴 시 무효다. const 액터에서도 자식을 수정할 수 있다.
	 */
	[[nodiscard]] AActor* GetChild(int32 index) const;

	/**
	 * 월드에 부모 변경을 위임한다. nullptr는 루트 분리이며 소유권은 유지한다.
	 * 기본 KeepLocal은 로컬 값을 유지하고 KeepWorld는 변경 전 월드 값으로 로컬을 다시 계산한다.
	 * 같은 월드에 등록된 살아 있는 액터가 대상이며, false이면 기존 관계와 로컬 값을 유지한다.
	 */
	[[nodiscard]] bool SetParent(AActor* newParent, EActorTransformRule rule = EActorTransformRule::KeepLocal);

	/**
	 * 월드에 자신과 모든 자손의 제거를 요청한다. 미등록이면 false이며 기존 상태를 유지한다.
	 * Tick 중 true는 요청 보관 성공이며, 보류 중 부모 변경을 허용하고 종료 때 현재 자손과 함께 파괴한다.
	 * 직접 요청된 자신의 남은 Tick은 건너뛰되 이미 실행 중인 Tick은 중단하지 않는다. 자손의 Tick은 직접 요청된 경우만 생략한다.
	 * NOTE: Tick 밖에서는 반환 전에 자신이 즉시 파괴되므로 이후 멤버에 접근하면 안 된다.
	 * 실제 파괴 뒤 자신과 제거된 자손/컴포넌트에 접근하면 안 된다.
	 */
	[[nodiscard]] bool Destroy();

	/** 실제 제거 처리가 시작됐는지 반환한다. 요청만 보류된 상태에서는 false다. */
	[[nodiscard]] bool IsActorBeingDestroyed() const;

	/**
	 * 소속이 자신인 컴포넌트의 소유권을 받는다. 성공 시 입력 포인터는 비워진다.
	 * 빈 포인터·다른 소속·복수 불허인 같은 실제 타입의 중복이면 false이며 입력과 기존 목록을 유지한다.
	 * 복수 허용 정책은 실제 객체에서 조회한다. 서로 다른 파생 타입은 별개로 취급한다.
	 * 메모리 부족은 false 반환으로 복구하지 않는다. 이벤트 호출은 없다.
	 */
	[[nodiscard]] bool AddComponent(TUniquePtr<UActorComponent>& component);

	/**
	 * T로 변환 가능한 첫 컴포넌트를 반환하며 없으면 nullptr다. 검색은 자기 목록에 한정한다.
	 * T는 UActorComponent 자신 또는 public 파생 타입이어야 한다.
	 * 반환 포인터는 비소유이며 컴포넌트 파괴 시 무효다. const 액터에서도 컴포넌트를 수정할 수 있다.
	 */
	template<typename T>
		requires std::derived_from<T, UActorComponent>
	[[nodiscard]] T* GetComponent() const;

	/**
	 * 자기 목록에서 T와 파생 타입의 모든 컴포넌트를 추가 순서대로 조회한다.
	 * T는 UActorComponent 자신 또는 public 파생 타입이어야 한다.
	 * 출력의 기존 내용을 비우고 채우며 확보된 공간은 재사용한다. 일치가 없으면 빈 배열이다.
	 * 포인터는 비소유이며 컴포넌트 파괴 시 무효다. const 액터에서도 조회한 컴포넌트는 수정할 수 있다.
	 */
	template<typename T>
		requires std::derived_from<T, UActorComponent>
	void GetComponents(TArray<T*>& outComponents) const;

protected:
	/**
	 * 첫 World Tick 직전에 호출할 시작 처리를 재정의한다.
	 * NOTE: 기반 BeginPlay를 호출하지 않으면 시작 미완료 상태로 남아 Tick이 보류된다.
	 */
	virtual void BeginPlay();

	/** World가 실제 제거 전에 호출할 종료 처리를 재정의한다. 기반 함수를 호출해 시작 완료 상태를 해제한다. */
	virtual void EndPlay(EEndPlayReason reason);

private:
	enum class EActorBeginPlayState : uint8
	{
		HasNotBegunPlay,
		BeginningPlay,
		HasBegunPlay
	};

	friend class UWorld;

	/** World가 등록된 액터의 첫 Tick 전에 호출한다. 시작 전일 때만 진입하며 완료 상태는 기반 BeginPlay가 설정한다. */
	void DispatchBeginPlay();

	/** World가 관계·등록 정리 뒤 파괴 전에 호출한다. 시작 완료 상태인 경우에만 가상 종료 처리를 실행한다. */
	void RouteEndPlay(EEndPlayReason reason);

	UWorld* const world;
	AActor* parent = nullptr;
	bool bActorIsBeingDestroyed = false;
	EActorBeginPlayState actorHasBegunPlay = EActorBeginPlayState::HasNotBegunPlay;
	TArray<TUniquePtr<UActorComponent>> components{};
	TArray<AActor*> children{};
};

template<typename T>
	requires std::derived_from<T, UActorComponent>
T* AActor::GetComponent() const
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

template<typename T>
	requires std::derived_from<T, UActorComponent>
void AActor::GetComponents(TArray<T*>& outComponents) const
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
