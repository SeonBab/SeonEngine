#pragma once

#include "Core/CoreTypes.h"
#include "Object.h"
#include "PlayState.h"

class AActor;

/** 액터와 컴포넌트의 플레이 종료 이유다. 객체 파괴와 실행 종료를 구분한다. */
enum class EEndPlayReason : uint8
{
	Destroyed,
	Quit
};

////////////////////////////////////////////////////////////////////////////////
// 액터에 붙는 기능 객체의 공통 기반이다.
// 소속 액터를 소유하지 않으며 액터는 컴포넌트보다 오래 살아 있어야 한다.
////////////////////////////////////////////////////////////////////////////////
class UActorComponent : public UObject
{
#pragma region Lifecycle

public:
	/** 소속 액터를 지정한다. 생성 후 소속을 변경하지 않는다. */
	explicit UActorComponent(AActor& owner);

	/** 기반 포인터를 통한 파괴에서도 파생 컴포넌트의 소멸자를 실행한다. */
	~UActorComponent() override;

	UActorComponent(const UActorComponent&) = delete;
	UActorComponent& operator=(const UActorComponent&) = delete;
	UActorComponent(UActorComponent&&) = delete;
	UActorComponent& operator=(UActorComponent&&) = delete;

#pragma endregion

#pragma region ActorIntegration

public:
	/** 소속 액터를 반환한다. 액터의 수명을 연장하지 않는다. */
	AActor& GetOwner() const { return owner; }

	/** 같은 실제 타입의 복수 추가 허용 정책이다. 기본은 false이며 파생 타입은 타입별 고정 정책으로 재정의한다. */
	virtual bool AllowsMultipleComponents() const;

private:
	AActor& owner;

#pragma endregion

#pragma region PlayLifecycle

public:
	/** 기반 BeginPlay에서 기록한 시작 완료 상태를 반환한다. 상태 변경이나 이벤트 호출은 하지 않는다. */
	[[nodiscard]] bool HasBegunPlay() const;

	/** 시작 완료 상태를 기록한다. 파생 구현은 기반 함수를 호출해야 하며 소유 액터의 기반 BeginPlay에서 호출한다. */
	virtual void BeginPlay();

	/** 시작 완료 상태를 해제한다. 파생 구현은 기반 함수를 호출해야 하며 시작한 경우 소유 액터의 기반 EndPlay에서 호출한다. */
	virtual void EndPlay(EEndPlayReason reason);

	/** World의 컴포넌트 Tick 전달 여부만 설정한다. 기본은 true이며 시작·종료 상태는 바꾸지 않는다. 같은 스레드에서 호출한다. */
	void SetComponentTickEnabled(bool enabled);

	/** 시작 완료 여부와 별개인 Tick 활성 설정을 반환한다. */
	[[nodiscard]] bool IsComponentTickEnabled() const;

	/** World가 소유 액터 Tick 뒤 시작했고 Tick이 활성인 컴포넌트에 전달한다. 인수는 초 단위이며 기본 구현은 비어 있다. */
	virtual void TickComponent(float deltaSeconds);

private:
	EPlayState playState = EPlayState::NotBegun; // 현재는 NotBegun/Begun만 사용하며 기반 이벤트가 전환한다.
	bool bTickEnabled = true;                    // World가 각 Tick 호출 직전에 검사한다. 실행 중인 콜백은 중단하지 않는다.

#pragma endregion
};
