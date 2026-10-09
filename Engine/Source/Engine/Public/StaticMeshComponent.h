#pragma once

#include "ActorComponent.h"
#include "Containers/Array.h"
#include "Math/Vector3.h"

class FActor;

////////////////////////////////////////////////////////////////////////////////
// 스태틱 메시 표시 기능의 컴포넌트다.
// 로컬 정점 위치를 보관하며 렌더 씬 등록은 아직 없다.
////////////////////////////////////////////////////////////////////////////////
class FStaticMeshComponent : public FActorComponent
{
public:
	/** 소속 액터를 지정한다. 생성만으로 액터 목록에 추가하거나 렌더 씬에 등록하지 않는다. */
	explicit FStaticMeshComponent(FActor& ownerActor);

private:
	// TODO(seon): 공유 메시 데이터 타입을 도입하면 정점 저장을 그 타입으로 이전하고 컴포넌트에는 메시 참조를 남긴다.
	// 컴포넌트 로컬 공간의 정점 위치다. 생성 시 빈 배열이다.
	TArray<FVector3> vertices;
};
