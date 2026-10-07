#pragma once

#include "ActorComponent.h"
#include "Math/Quat.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"
#include "Math/Vector3.h"

class FActor;

////////////////////////////////////////////////////////////////////////////////
// 액터의 위치, 회전, 크기를 보관하는 컴포넌트다.
// 액터 생성 시 하나가 만들어진다. 현재는 데이터 보관만 제공하며 좌표 변환은 아직 없다.
////////////////////////////////////////////////////////////////////////////////
class FTransformComponent : public FActorComponent
{
public:
	/** 소속 액터를 지정하고 위치 0 / 회전 없음 / 크기 1배로 초기화한다. */
	explicit FTransformComponent(FActor& ownerActor);

	/** 저장된 위치 / 회전 / 크기를 묶은 트랜스폼의 복사본을 반환한다. */
	[[nodiscard]] FTransform GetTransform() const;

	/** 저장된 위치를 cm 단위의 값으로 반환한다. */
	[[nodiscard]] FVector3 GetPosition() const;

	/** 저장된 위치를 cm 단위의 값으로 바꾼다. 좌표 변환과 시스템 씬 갱신은 아직 없다. */
	void SetPosition(FVector3 newPosition);

	/** 저장된 회전을 도 단위의 각도로 변환해 반환한다. */
	[[nodiscard]] FRotator GetRotation() const;

	/** 도 단위 각도를 쿼터니언으로 변환해 저장된 회전을 대체한다. */
	void SetRotation(FRotator newRotation);

	/** 유한한 단위 쿼터니언으로 저장된 회전을 대체한다. 검사와 정규화는 수행하지 않는다. */
	void SetRotation(FQuat newRotation);

	/** 저장된 회전 쿼터니언의 복사본을 반환한다. */
	[[nodiscard]] FQuat GetQuaternion() const;

	/** 저장된 축별 크기 배율을 값으로 반환한다. 기본값은 각 축 1배다. */
	[[nodiscard]] FVector3 GetScale() const;

	/** 저장된 축별 크기 배율을 새 값으로 바꾼다. 시스템 씬 갱신은 아직 없다. */
	void SetScale(FVector3 newScale);

private:
	FTransform transform{};
};
