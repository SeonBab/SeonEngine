#pragma once

#include "ActorComponent.h"
#include "Containers/Array.h"
#include "Math/Vector3.h"

class FActor;
struct FMeshRenderData;

////////////////////////////////////////////////////////////////////////////////
// 스태틱 메시 표시 기능의 컴포넌트다.
// 로컬 정점 위치를 보관하며 렌더 씬 등록은 아직 없다.
////////////////////////////////////////////////////////////////////////////////
class FStaticMeshComponent : public FActorComponent
{
public:
	/** 소속 액터를 지정한다. 생성만으로 액터 목록에 추가하거나 렌더 씬에 등록하지 않는다. */
	explicit FStaticMeshComponent(FActor& ownerActor);

	/** 로컬 정점 위치를 복사하여 기존 배열을 교체한다. 빈 입력은 비운다. GPU 갱신은 하지 않는다. */
	void SetVertices(const TArray<FVector3>& inVertices);

	/** 로컬 정점 배열을 읽기 전용 참조로 반환한다. 컴포넌트 파괴 후 무효이며 설정 뒤 원소 참조는 다시 얻는다. */
	const TArray<FVector3>& GetVertices() const;

	/**
	 * 로컬 정점 복사본과 소속 액터의 월드 변환 행렬을 값으로 반환한다. 빈 정점도 그대로 반환한다.
	 * 액터의 필수 트랜스폼과 유효한 변환 / 계층이 전제다. 씬 등록이나 GPU 갱신은 하지 않는다.
	 */
	FMeshRenderData GetRenderData() const;

private:
	// TODO(seon): 공유 메시 데이터 타입을 도입하면 정점 저장을 그 타입으로 이전하고 컴포넌트에는 메시 참조를 남긴다.
	// 컴포넌트 로컬 공간의 정점 위치다. 생성 시 빈 배열이다.
	TArray<FVector3> vertices;
};
