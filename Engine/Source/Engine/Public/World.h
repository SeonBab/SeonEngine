#pragma once

#include "Containers/Array.h"
#include "Templates/UniquePtr.h"

class FActor;
class UGameInstance;

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

	/** 기본 액터를 생성하여 소유한다. 반환 참조는 액터가 파괴되면 무효다. */
	FActor& SpawnActor();

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
	 * 프레임 루프에는 아직 연결하지 않았다.
	 */
	// HACK(seon): GC 또는 별도 수명 관리 시스템이 안전한 회수를 맡으면 이 함수와 호출부를 해당 시스템으로 대체하고 제거한다.
	void PurgeDestroyedActors();

	/** 엔진 실행 코드가 세션을 비소유 연결한다. nullptr는 연결 해제다. */
	void SetGameInstance(UGameInstance* inGameInstance);

	/** 비소유 세션을 반환한다. 연결 전 / 해제 후에는 nullptr다. */
	UGameInstance* GetGameInstance() const { return gameInstance; }

private:
	friend class FActor;

	void MarkActorIsBeingDestroyed(FActor& actor);

	TArray<TUniquePtr<FActor>> actors;
	// 논리적으로 제거한 액터를 임시 소유하여 실행 중인 코드가 돌아올 때까지 메모리를 유지한다.
	// HACK(seon): GC 또는 별도 수명 관리 시스템으로 소유권과 회수 책임을 이전한 뒤 이 보관 목록을 제거한다.
	TArray<TUniquePtr<FActor>> destroyedActors;
	UGameInstance* gameInstance = nullptr;
};
