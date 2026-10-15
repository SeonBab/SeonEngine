#pragma once

#include "ActorComponent.h"
#include "Math/Quat.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"
#include "Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 액터에 붙어 위치·회전·크기 데이터를 보관한다.
// SetRotation은 각도를 쿼터니언으로 변환한다. 좌표 공간·부모 관계는 아직 없다.
////////////////////////////////////////////////////////////////////////////////
class FTransformComponent : public FActorComponent
{
public:
	/** 소속 액터를 지정하고 위치 0·회전 없음·크기 1배로 초기화한다. 자동 등록은 하지 않는다. */
	explicit FTransformComponent(AActor& owner);

	/** 저장된 변환 데이터의 독립적인 복사본을 반환한다. */
	FTransform GetTransform() const;

	/** 데이터를 복사해 저장한다. 유효성 검사·정규화·좌표 변환은 수행하지 않는다. */
	void SetTransform(const FTransform& newTransform);

	/** 저장된 위치의 복사본을 반환한다. */
	FVector3 GetPosition() const;

	/** 위치만 복사해 저장한다. 좌표 변환이나 값 검사는 수행하지 않는다. */
	void SetPosition(FVector3 newPosition);

	/** 저장된 회전 쿼터니언의 복사본을 반환한다. */
	FQuat GetQuaternion() const;

	/** 회전 쿼터니언만 복사해 저장한다. 각도 변환이나 정규화는 수행하지 않는다. */
	void SetQuaternion(FQuat newRotation);

	/** 유한하고 정규화된 저장 회전을 도 단위 대표 각도로 반환한다. 저장 값은 변경하지 않는다. */
	FRotator GetRotation() const;

	/** 유한한 도 단위 각도를 정규화된 쿼터니언으로 바꿔 회전만 저장한다. */
	void SetRotation(FRotator newRotation);

	/** 저장된 축별 크기 배율의 복사본을 반환한다. */
	FVector3 GetScale() const;

	/** 축별 크기 배율만 복사해 저장한다. 값 검사나 변환 계산은 수행하지 않는다. */
	void SetScale(FVector3 newScale);

private:
	FTransform transform{};
};
