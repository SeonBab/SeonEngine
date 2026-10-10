#pragma once

#include "Containers/Array.h"
#include "Templates/UniquePtr.h"

class FActor;
class FStaticMeshComponent;
class UGameInstance;
struct FMeshRenderData;

////////////////////////////////////////////////////////////////////////////////
// 게임 객체와 시스템 씬을 관리할 월드의 기반이다.
// 액터를 생성하여 소유한다. 시스템 씬은 아직 없고 게임 세션은 비소유로 연결한다.
////////////////////////////////////////////////////////////////////////////////
class FWorld
{
public:
	/** 빈 액터 목록을 준비한다. 시스템 씬 생성은 아직 구현하지 않았다. */
	FWorld();

	/** 소유한 액터를 파괴한다. 시스템 씬 등록 해제는 아직 연결하지 않았다. */
	~FWorld();

	/**
	 * 전달받은 시간 간격(초)을 저장하고 월드 게임 시간에 누적한다. 0은 정상 입력이다.
	 * deltaSeconds는 유한한 0 이상이어야 하며 전제 위반은 Release에서도 중단한다.
	 * 시간 갱신 뒤 시작 시점의 루트와 자손을 한 번씩 갱신한다. 중간 생성은 다음 호출, 제거 중은 생략한다.
	 * 갱신 중 실제 회수와 중첩 Tick은 금지한다. 일시정지 / 시간 배율은 아직 없다.
	 */
	void Tick(float deltaSeconds);

	/** 기본 액터를 생성하여 소유한다. 반환 참조는 액터가 파괴되면 무효다. */
	FActor& SpawnActor();

	/** 루트와 자손의 스태틱 메시 컴포넌트 주소를 수집한다. 출력은 비우며 비소유다. 렌더 등록 / 그리기는 하지 않는다. */
	void CollectStaticMeshComponents(TArray<FStaticMeshComponent*>& outComponents) const;

	/**
	 * 제거 중 액터를 제외한 루트 / 자손 메시의 정점 복사본과 월드 행렬을 수집한다. 기존 출력은 비운다.
	 * 빈 메시도 포함하며 결과는 값을 소유한다. 수집 중 객체 / 계층 변경이나 파괴는 금지한다.
	 * 컴포넌트의 유효한 변환 / 계층이 전제이며 렌더러 전달과 GPU 갱신은 하지 않는다.
	 */
	void CollectMeshRenderData(TArray<FMeshRenderData>& outRenderData) const;

	/**
	 * 이 월드의 액터와 자손을 제거 처리한다. actor는 살아 있는 비소유 주소여야 한다.
	 * true는 제거 처리 시작 또는 이미 제거 중이다. 메모리의 즉시 파괴를 뜻하지 않는다.
	 * actor가 null이거나 다른 월드 / 미등록이면 호출 전제 위반으로 실행을 중단한다.
	 * 보관한 메모리는 PurgeDestroyedActors 또는 월드 소멸 때 회수한다.
	 */
	[[nodiscard]] bool DestroyActor(FActor* actor);

	/**
	 * 보관한 제거 액터와 자손을 실제로 파괴한다. 기존 포인터 / 참조는 이후 무효다.
	 * 해당 객체를 사용하는 호출과 갱신 순회가 모두 끝난 안전한 시점에만 호출한다.
	 * 게임 엔진이 월드 Tick 반환 직후 호출한다. 외부에서 직접 호출할 때도 안전한 시점이어야 한다.
	 */
	// HACK(seon): GC 또는 별도 수명 관리 시스템이 안전한 회수를 맡으면 이 함수와 호출부를 해당 시스템으로 대체하고 제거한다.
	void PurgeDestroyedActors();

	/** 엔진 실행 코드가 세션을 비소유 연결한다. nullptr는 연결 해제다. */
	void SetGameInstance(UGameInstance* inGameInstance);

	/** 비소유 세션을 반환한다. 연결 전 / 해제 후에는 nullptr다. */
	UGameInstance* GetGameInstance() const { return gameInstance; }

	/** 마지막 Tick의 시간 간격(초)을 값으로 반환한다. 첫 Tick 전에는 0이다. */
	float GetDeltaSeconds() const;

	/** Tick으로 누적한 월드 게임 시간(초)을 값으로 반환한다. 생성 시 0이다. */
	double GetTimeSeconds() const;

private:
	friend class FActor;

	/** 출력 목록을 비우고 루트와 모든 자손의 비소유 주소를 수집한다. Tick은 호출하지 않는다. */
	void CollectActors(TArray<FActor*>& outActors) const;

	void MarkActorIsBeingDestroyed(FActor& actor);

	// 마지막 Tick에서 전달받은 시간 간격(초)이다. 첫 Tick 전에는 0이다.
	float DeltaTimeSeconds = 0.0f;
	// Tick으로 누적한 월드 게임 시간(초)이다. 생성 시 0이다.
	double TimeSeconds = 0.0;

	TArray<TUniquePtr<FActor>> actors;
	// 논리적으로 제거한 액터를 임시 소유하여 실행 중인 코드가 돌아올 때까지 메모리를 유지한다.
	// HACK(seon): GC 또는 별도 수명 관리 시스템으로 소유권과 회수 책임을 이전한 뒤 이 보관 목록을 제거한다.
	TArray<TUniquePtr<FActor>> destroyedActors;
	UGameInstance* gameInstance = nullptr;
};
