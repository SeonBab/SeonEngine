#pragma once

#include "Actor.h"
#include "Core/Containers/Array.h"
#include "Core/Templates/UniquePtr.h"
#include "Object.h"
#include "Renderer/Scene.h"

#include <concepts>
#include <utility>

class FInputState;
class UCameraComponent;
class UPlayerInput;

////////////////////////////////////////////////////////////////////////////////
// 등록된 모든 액터와 렌더링 Scene을 단독 소유하고 부모와 자식의 관계 변경을 통제한다.
// 부모 변경은 로컬 유지 또는 월드 기준 재계산을 선택하며 개별 제거는 자손도 함께 파괴한다.
////////////////////////////////////////////////////////////////////////////////
class UWorld : public UObject
{
public:
	/** 등록된 액터와 렌더링 항목이 없는 월드를 만든다. GPU 자원은 생성하지 않는다. */
	UWorld();

	/** 남은 플레이를 Quit로 종료한 뒤 등록 액터와 Scene을 정리한다. 명시적 EndPlay 뒤에는 종료 알림을 반복하지 않는다. */
	~UWorld() override;

	UWorld(const UWorld&) = delete;
	UWorld& operator=(const UWorld&) = delete;
	UWorld(UWorld&&) = delete;
	UWorld& operator=(UWorld&&) = delete;

	/** 소유한 Scene을 빌린다. 렌더러는 연결하지 않으며 World 소멸 후 참조는 무효다. */
	[[nodiscard]] FScene& GetScene();

	/** 소유한 Scene을 읽기 전용으로 빌린다. World 소멸 후 참조는 무효다. */
	[[nodiscard]] const FScene& GetScene() const;

	/**
	 * 이 월드에 등록된 카메라 액터를 비소유로 선택한다. nullptr는 선택 해제다.
	 * 미등록, 다른 월드, 제거 중, 카메라 없음은 false이며 기존 선택을 유지한다.
	 * 제거 요청만 보류된 대상은 선택할 수 있으며 실제 제거 전에 선택을 해제한다.
	 * 인수는 null 또는 살아 있는 객체여야 한다. 같은 대상의 재선택도 true다.
	 */
	[[nodiscard]] bool SetActiveCamera(AActor* actor);

	/**
	 * 선택한 액터에서 첫 카메라 컴포넌트와 파생 타입을 조회한다. 선택이 없으면 nullptr다.
	 * 반환 포인터는 비소유이며 실제 액터 파괴 뒤 무효다. const 월드에서도 수정할 수 있다.
	 */
	[[nodiscard]] UCameraComponent* GetActiveCamera() const;

	/**
	 * 현재 등록된 액터에 시작 알림을 보낸 뒤 월드 플레이를 시작한다. 같은 스레드의 프레임 밖에서 호출한다.
	 * 이미 시작했거나 종료된 월드, 액터 콜백 순회 중의 중첩 호출은 무시한다.
	 * 알림 중 제거는 보류하고 순회 뒤 처리한다. 알림 중 생성한 액터는 다음 Tick에서 시작한다.
	 */
	void BeginPlay();

	/** 초기 액터의 시작 알림과 보류 제거 처리를 마친 플레이 중 상태를 반환한다. */
	[[nodiscard]] bool HasBegunPlay() const;

	/**
	 * BeginPlay가 완료된 월드에서 등록 순서로 입력을 전달한 뒤 Tick한다. 시작 전, 중첩 호출과 종료 중·종료 후 호출은 무시한다.
	 * 도중 생성한 액터는 다음 호출부터 갱신하고 제거 중인 대상은 건너뛴다. 시간값은 그대로 전달한다.
	 * 순회 중에는 요청 대상만 보관하며 직접 요청된 대상의 남은 Tick은 건너뛴다. 이미 실행 중인 Tick은 중단하지 않는다.
	 * 플레이 중 추가한 액터는 첫 Tick 전에 시작 처리를 한 번 호출한다. 기반 BeginPlay 누락으로 미완료면 Tick을 보류하고 시작을 재호출하지 않는다.
	 * 시작 처리 뒤 직접 제거 요청을 다시 확인해 해당 대상의 Tick은 생략한다.
	 * 입력 목록은 액터의 시작 처리 완료 뒤 수집하며 해당 처리 중 추가한 컴포넌트는 다음 호출부터 반영한다.
	 * 입력 중 자기 제거 요청은 현재 키 처리/액션 전달이 반환한 뒤 남은 입력 처리와 Tick을 생략한다.
	 * 입력과 관리자는 이번 호출 동안만 빌리며 저장하지 않는다. 콜백도 기존 Tick의 제거 보류 범위 안에서 실행한다.
	 * 호출 전에 액션 평가가 성공해야 하며 호출 동안 평가 결과/등록 목록/Context를 외부에서 바꾸면 안 된다.
	 * 자손은 직접 요청되지 않았다면 Tick하며 종료 정리에서 처리 시점의 현재 자손을 함께 파괴한다.
	 */
	void Tick(float deltaSeconds, const FInputState& input, UPlayerInput& playerInput);

	/**
	 * 시작한 등록 액터에 종료 알림을 보낸다. 종료 후 Tick은 무시하며 같은 World를 다시 시작하지 않는다.
	 * 시작 알림과 Tick 밖에서 호출한다. 해당 순회 중 요청과 반복 호출·종료 알림 중 재진입은 무시하며 요청을 보관하지 않는다.
	 * 종료 알림 중 제거는 보류했다가 모든 알림 뒤 처리한다. 나머지 액터는 소유·관계를 유지하고 World 파괴 때 해제한다.
	 * 알림 중 생성한 액터는 시작하지 않으며 별도 종료 알림 없이 이후 정리한다. 같은 스레드에서 호출한다.
	 */
	void EndPlay(EEndPlayReason reason);

	/**
	 * 초기 구성 후 액터 등록 순서대로 URenderStateComponent의 미생성 렌더 상태를 만든다.
	 * 각 액터에서는 종류 구분 없이 컴포넌트 추가 순서를 따른다. 준비된 메시 자산/재질을 사용하고 GPU 생성은 하지 않는다.
	 * 생성된 상태는 변경하지 않는다. 실패하면 false로 중단하고 앞서 성공한 등록은 유지한다.
	 * Tick과 생성 및 제거 콜백 밖에서 호출한다.
	 * NOTE: 초기 준비 뒤 추가한 렌더 상태 컴포넌트는 자동으로 렌더 등록되지 않는다.
	 * NOTE: 컴포넌트 등록 수명에 통합할 때 초기 일괄 준비 함수와 호출의 존치 여부를 재검토한다.
	 */
	[[nodiscard]] bool CreateRenderStates();

	/**
	 * Tick 종료 후 그리기 전에 액터 등록 순서대로 생성된 렌더 상태를 갱신한다.
	 * 각 액터에서는 종류 구분 없이 URenderStateComponent를 컴포넌트 추가 순서대로 갱신한다.
	 * 각 컴포넌트의 SendRenderUpdates를 호출한다. 광원은 Renderer 없이 컴포넌트 소유 Proxy의 CPU 값만 갱신한다.
	 * 미생성 상태는 건너뛰며 자동 생성하지 않는다. 실패하면 false로 중단하고 앞서 성공한 갱신은 유지한다.
	 * Tick과 생성 및 제거 콜백 밖에서 명시 호출한다. 빈 월드와 갱신 대상이 없는 경우는 true다.
	 */
	[[nodiscard]] bool SendAllEndOfFrameUpdates();

	/**
	 * 이 월드를 전달해 액터를 생성하고 부모 없는 액터로 소유 목록에 등록한다. 생성자는 등록 전에 실행된다.
	 * T는 AActor 자신 또는 public 파생 타입이며 UWorld&로 생성할 수 있어야 한다.
	 * 반환 참조는 비소유이며 월드가 소멸하면 무효다. 할당 실패 복구, 생명주기 이벤트는 제공하지 않는다.
	 * 종료 중·종료 후 생성도 허용하지만 해당 액터의 플레이는 시작하지 않는다.
	 */
	template<typename T>
		requires std::derived_from<T, AActor> && std::constructible_from<T, UWorld&>
	[[nodiscard]] T& SpawnActor();

	/**
	 * 출력 배열을 비우고 현재 등록된 모든 액터를 등록 순서대로 채운다. 배열 용량은 재사용한다.
	 * 제거 요청이 보류된 액터도 실제 등록 해제 전까지 포함하며 소유권은 이전하지 않는다.
	 * const 월드에서도 수정 가능한 비소유 포인터를 반환한다. 실제 파괴 뒤 주소는 무효다.
	 */
	void GetActors(TArray<AActor*>& outActors) const;

	/**
	 * 등록된 액터의 부모를 변경한다. nullptr 부모는 루트 분리이며 같은 부모 요청은 변경 없이 true다.
	 * 기본 KeepLocal은 로컬 값을 유지한다. KeepWorld는 변경 전 월드 값으로 새 로컬을 계산하며 정확 재현은 보장하지 않는다.
	 * null 대상, 미등록, 다른 월드, 자기/자손 연결은 false이며 기존 관계, 소유권, 저장 Transform을 유지한다.
	 * 자신이나 새 부모가 제거 중인 요청도 같은 부모/분리 여부와 무관하게 false다.
	 * 제거 요청만 보류된 대상은 기존 규칙대로 부모를 변경할 수 있다.
	 * 인수는 null 또는 살아 있는 객체여야 한다. true는 관계 처리 성공이며 이벤트, 할당 실패 복구는 제공하지 않는다.
	 */
	[[nodiscard]] bool SetActorParent(AActor* actor, AActor* newParent, EActorTransformRule rule = EActorTransformRule::KeepLocal);

	/**
	 * 등록된 액터와 처리 시점의 모든 자손을 제거한다. 월드 시작·종료 알림 또는 Tick 중에는 요청만 보관하고 순회 뒤 파괴한다.
	 * 보류 중에는 제거 상태를 켜지 않고 부모 변경을 허용하되 직접 요청된 대상의 남은 Tick은 건너뛴다.
	 * 같은 대상의 중복 요청은 true다.
	 * 월드 시작·종료 알림과 Tick 순회 밖에서는 관계/등록 정리 후 자손부터 즉시 파괴한다.
	 * null, 미등록, 다른 월드 대상은 false다. 인수는 null 또는 살아 있는 객체여야 한다.
	 * 소멸 중 다른 등록 액터의 제거 요청을 허용한다. 이미 분리한 제거 대상은 미등록으로 거부한다.
	 * 월드 종료 전에는 관계/등록 정리 뒤 시작 완료 대상에 EndPlay(Destroyed)를 호출하고 파괴한다.
	 * 기반 Actor.EndPlay가 시작 상태를 해제한 대상은 알림을 반복하지 않고 파괴한다.
	 * 실제 파괴 후 제거된 액터/컴포넌트의 외부 포인터는 무효다. GC, 할당 실패 복구는 없다.
	 */
	[[nodiscard]] bool DestroyActor(AActor* actor);

private:
	/** 직접 제거 요청이 보류된 대상인지 주소로 확인한다. 객체를 역참조하지 않는다. */
	[[nodiscard]] bool IsActorPendingDestroy(const AActor* actor) const;

	/** 등록된 대상의 관계/소유를 정리하고 실제 파괴한다. Tick 순회가 끝난 안전한 구간에서만 호출한다. */
	bool DestroyActorImmediately(AActor* actor, EEndPlayReason reason = EEndPlayReason::Destroyed);

	/** 보관한 요청을 처리한다. 실제 파괴 전에 함께 제거되는 자손의 요청도 제외하며 추가 요청도 이어서 처리한다. */
	void PurgeDestroyedActors();

	// NOTE: 액터, 컴포넌트 정리 중 Scene을 사용하므로 actors보다 먼저 선언해야 한다.
	FScene scene;
	TArray<TUniquePtr<AActor>> actors{};
	AActor* activeCameraActor = nullptr;
	TArray<AActor*> pendingDestroyActors{};
	bool bIsTicking = false;     // 액터 시작·갱신과 보류한 제거를 처리하는 중이다.
	bool bBegunPlay = false;     // 초기 시작 알림이 완료됐으며 플레이가 아직 끝나지 않았다.
	bool bIsTearingDown = false; // 월드 종료가 시작됐으며 이후 Tick을 막는다.
};

template<typename T>
	requires std::derived_from<T, AActor> && std::constructible_from<T, UWorld&>
T& UWorld::SpawnActor()
{
	TUniquePtr<T> actor = MakeUnique<T>(*this);
	T* spawnedActor = actor.get();
	actors.push_back(std::move(actor));

	return *spawnedActor;
}
