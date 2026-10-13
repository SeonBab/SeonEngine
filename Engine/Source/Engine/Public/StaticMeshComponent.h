#pragma once

#include "ActorComponent.h"
#include "Templates/SharedPointer.h"

class FActor;
class UStaticMesh;
struct FMeshRenderData;

////////////////////////////////////////////////////////////////////////////////
// 스태틱 메시 표시 기능의 컴포넌트다.
// 메시 자산을 공유 소유하고 그 형상과 소속 액터의 월드 변환을 렌더 데이터로 전달한다.
////////////////////////////////////////////////////////////////////////////////
class FStaticMeshComponent : public FActorComponent
{
public:
	/** 소속 액터를 지정한다. 생성만으로 액터 목록에 추가하거나 렌더 씬에 등록하지 않는다. */
	explicit FStaticMeshComponent(FActor& ownerActor);

	/**
	 * 사용할 메시 자산의 공유 참조를 지정한다. 형상을 복사하거나 GPU 상태를 갱신하지 않는다.
	 * 다음 GetRenderData 조회는 지정 자산의 형상을 복사한다. GPU 상태를 직접 갱신하지 않는다.
	 * @param inStaticMesh nullptr는 이 컴포넌트의 메시 참조를 해제한다. 다른 소유자의 참조는 유지한다.
	 * @return 다른 메시로 변경하거나 기존 참조를 해제하면 true. 같은 메시 재지정은 false이며 오류가 아니다.
	 */
	bool SetStaticMesh(TSharedPtr<UStaticMesh> inStaticMesh);

	/**
	 * 지정된 메시를 비소유 포인터로 조회한다. 조회는 공유 참조 수나 렌더 상태를 바꾸지 않는다.
	 * 자산 수명을 연장하지 않는다. 교체 / 해제 / 컴포넌트 파괴 후에는 이전 포인터의 유효성을 가정하지 않는다.
	 * @return 메시가 없으면 nullptr. const 컴포넌트에서도 수정 가능한 자산을 반환하며 직접 삭제하면 안 된다.
	 */
	UStaticMesh* GetStaticMesh() const;

	/**
	 * 지정 자산의 정점 / 연결 규칙과 소속 액터의 월드 변환을 값으로 반환한다. 자산이 없으면 빈 정점과 기본 List다.
	 * 반환 복사본은 이후 자산 변경 / 파괴와 독립적이다. 조회 중 자산·액터를 변경하거나 파괴하지 않는다.
	 * 액터의 필수 트랜스폼과 유효한 변환 / 계층이 전제다. 씬 등록이나 GPU 갱신은 하지 않는다.
	 */
	FMeshRenderData GetRenderData() const;

private:
	// 메시를 사용하는 동안 수명을 유지한다. 초기값은 비어 있고 교체 / 해제 / 컴포넌트 파괴 시 참조를 놓는다.
	// 마지막 공유 소유자가 놓으면 자산이 파괴된다. 자원 관리자 / 핸들은 후속이다.
	TSharedPtr<UStaticMesh> staticMesh;
};
