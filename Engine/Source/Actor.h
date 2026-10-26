#pragma once

#include "Components/ActorComponent.h"
#include "Core/Containers/Array.h"
#include "Core/Containers/SeonString.h"
#include "Core/CoreTypes.h"
#include "Core/Templates/UniquePtr.h"
#include "Object.h"
#include "PlayState.h"

#include <concepts>

class UWorld;

/** 부모 변경 시 유지할 변환 공간을 지정한다. KeepWorld의 표현 한계는 FTransform 상대 변환을 따른다. */
enum class EActorTransformRule : uint8
{
	KeepLocal,
	KeepWorld
};

////////////////////////////////////////////////////////////////////////////////
// 컴포넌트를 단독 소유하여 게임 객체 하나를 구성한다.
// 소속이 자신인 컴포넌트를 받아 보관하고 소멸 시 함께 정리한다.
////////////////////////////////////////////////////////////////////////////////
class AActor : public UObject
{
	friend class UWorld;

#pragma region Lifecycle

public:
	/** 비소유 월드를 연결하고 기본 트랜스폼 하나를 소유한다. 자동 등록은 하지 않으며 월드는 액터보다 오래 살아야 한다. */
	explicit AActor(UWorld& inWorld);

	/** 기반 포인터를 통한 파생 액터 파괴를 지원하고 소유한 컴포넌트를 정리한다. 자식 관계는 비소유다. */
	~AActor() override;

	AActor(const AActor&) = delete;
	AActor& operator=(const AActor&) = delete;
	AActor(AActor&&) = delete;
	AActor& operator=(AActor&&) = delete;

#pragma endregion

#pragma region WorldAndHierarchy

public:
	/** 생성 시 지정한 월드를 반환한다. 월드의 수명을 연장하거나 소유 목록 등록 여부를 검사하지 않는다. */
	[[nodiscard]] UWorld& GetWorld() const;

	/** 비소유 부모를 반환한다. 부모가 없으면 nullptr다. */
	[[nodiscard]] AActor* GetParent() const;

	/** 직접 연결된 자식 수를 반환한다. 손자 등 후손은 포함하지 않는다. */
	[[nodiscard]] int32 GetChildCount() const;

	/**
	 * 직접 연결된 자식을 반환하며 음수, 범위 밖 인덱스는 nullptr다.
	 * 반환 포인터는 비소유이며 자식 파괴 시 무효다. const 액터에서도 자식을 수정할 수 있다.
	 */
	[[nodiscard]] AActor* GetChild(int32 index) const;

	/**
	 * 월드에 부모 변경을 위임한다. nullptr는 루트 분리이며 소유권은 유지한다.
	 * 기본 KeepLocal은 로컬 값을 유지하고 KeepWorld는 변경 전 월드 값으로 로컬을 다시 계산한다.
	 * 같은 월드에 등록된 살아 있는 액터가 대상이며, false이면 기존 관계와 로컬 값을 유지한다.
	 */
	[[nodiscard]] bool SetParent(AActor* newParent, EActorTransformRule rule = EActorTransformRule::KeepLocal);

#pragma endregion

#pragma region Destruction

public:
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

#pragma endregion

#pragma region PlayLifecycle

public:
	/** World의 Actor Tick 전달 여부만 설정한다. 기본은 true이며 입력·컴포넌트 Tick·시작·종료는 제어하지 않는다. 같은 스레드에서 호출한다. */
	void SetActorTickEnabled(bool enabled);

	/** 시작 완료 여부와 별개인 Actor Tick 활성 설정을 반환한다. */
	[[nodiscard]] bool IsActorTickEnabled() const;

	/** World가 시작했고 Actor Tick이 활성인 대상에 전달한다. 초 단위 프레임 경과 시간을 받으며 기본 구현은 비어 있다. */
	virtual void Tick(float deltaSeconds);

	/** 기반 BeginPlay에서 시작 완료 상태로 전환됐는지 반환한다. */
	[[nodiscard]] bool HasActorBegunPlay() const;

	/** 시작 진입 뒤 기반 BeginPlay가 아직 완료 상태로 바꾸지 않았는지 반환한다. 기반 호출을 생략하면 true가 유지된다. */
	[[nodiscard]] bool IsActorBeginningPlay() const;

protected:
	/**
	 * 첫 World Tick 직전에 호출할 시작 처리를 재정의한다.
	 * 기반 구현은 호출 시점 목록의 미시작 컴포넌트를 시작한 뒤 액터를 시작 완료로 표시한다.
	 * NOTE: 기반 BeginPlay를 호출하지 않으면 시작 미완료 상태로 남아 Tick이 보류된다.
	 */
	virtual void BeginPlay();

	/**
	 * World가 실제 제거 전에 호출할 종료 처리를 재정의한다. 파생 구현은 기반 함수를 호출해야 한다.
	 * 기반 구현은 시작 완료 상태를 먼저 해제하고 호출 시점 목록의 시작한 컴포넌트에 같은 종료 이유를 전달한다.
	 */
	virtual void EndPlay(EEndPlayReason reason);

private:
	/** World가 등록된 액터의 첫 Tick 전에 호출한다. 시작 전일 때만 진입하며 완료 상태는 기반 BeginPlay가 설정한다. */
	void DispatchBeginPlay();

	/** World가 관계와 등록을 정리한 뒤 파괴 전에 호출한다. 시작 완료 상태인 경우에만 가상 종료 처리를 실행한다. */
	void RouteEndPlay(EEndPlayReason reason);

#pragma endregion

#pragma region Components

public:
	/**
	 * 소속이 자신인 컴포넌트에 이름을 복사해 지정하고 소유권을 받는다. 성공 시 입력 포인터는 비워진다.
	 * 빈 이름, 같은 액터 안의 중복 이름, 빈 포인터, 다른 소속, 복수 불허인 같은 실제 타입의 중복이면 false다.
	 * 실패 시 입력 포인터와 이름, 기존 목록을 유지한다. 이름 비교는 대소문자를 구분한다.
	 * 복수 허용 정책은 실제 객체에서 조회한다. 서로 다른 파생 타입은 별개로 취급한다.
	 * 액터가 시작 중이거나 시작 완료면 소유권을 받은 뒤 미시작 컴포넌트의 BeginPlay를 반환 전에 호출한다.
	 * 필요한 설정은 추가 전에 마친다. true는 소유권 이전 성공이며 콜백 후 객체의 생존을 보장하지 않는다.
	 * 콜백 중 Destroy의 제거 시점은 World 계약을 따르며 즉시 파괴 뒤 액터/컴포넌트에 접근하면 안 된다.
	 * 메모리 부족은 false 반환으로 복구하지 않는다.
	 */
	[[nodiscard]] bool AddComponent(FStringView name, TUniquePtr<UActorComponent>& component);

	/** 이름을 대소문자를 구분해 조회한다. 없으면 nullptr이며 반환 포인터는 비소유다. const 액터에서도 수정할 수 있고 파괴 후 무효다. */
	[[nodiscard]] UActorComponent* FindComponent(FStringView name) const;

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

#pragma endregion

#pragma region ActorState

private:
	UWorld* const world;
	AActor* parent = nullptr;
	bool bActorIsBeingDestroyed = false;
	EPlayState actorHasBegunPlay = EPlayState::NotBegun;
	bool bTickEnabled = true; // World가 Actor Tick 직전에 검사한다. 실행 중인 콜백은 중단하지 않는다.
	TArray<TUniquePtr<UActorComponent>> components{};
	TArray<AActor*> children{};

#pragma endregion
};

#pragma region Components

template<typename T>
	requires std::derived_from<T, UActorComponent>
T* AActor::GetComponent() const
{
	for (const TUniquePtr<UActorComponent>& component : components)
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

	for (const TUniquePtr<UActorComponent>& component : components)
	{
		if (T* found = dynamic_cast<T*>(component.get()))
		{
			outComponents.push_back(found);
		}
	}
}

#pragma endregion
