#pragma once

#include "ActorComponent.h"
#include "Math/Quat.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"
#include "Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 액터에 붙어 위치·회전·크기 데이터를 보관한다.
// 부모 기준 로컬 변환을 저장하고 부모 계층을 따라 월드 변환을 계산한다.
////////////////////////////////////////////////////////////////////////////////
class UTransformComponent : public UActorComponent
{
public:
	/** 소속 액터를 지정하고 위치 0·회전 없음·크기 1배로 초기화한다. 자동 등록은 하지 않는다. */
	explicit UTransformComponent(AActor& owner);

	/** 저장된 부모 기준 로컬 변환의 독립적인 복사본을 반환한다. 루트에서는 월드 변환과 같다. */
	FTransform GetTransform() const;

	/** 로컬 변환에 부모의 월드 변환을 합성해 반환한다. 저장 값 변경이나 캐시는 없다. */
	[[nodiscard]] FTransform GetComponentTransform() const;

	/** 데이터를 복사해 저장한다. 유효성 검사·정규화·좌표 변환은 수행하지 않는다. */
	void SetTransform(const FTransform& newTransform);

	/** 월드 변환을 부모 기준 로컬 값으로 계산해 저장한다. 표현 한계·대체 계산은 FTransform::GetRelativeTransform을 따른다. */
	void SetComponentTransform(const FTransform& newTransform);

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
