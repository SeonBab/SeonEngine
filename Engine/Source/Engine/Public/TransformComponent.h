#pragma once

#include "ActorComponent.h"
#include "Math/Quat.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"
#include "Math/Vector3.h"

class FActor;

////////////////////////////////////////////////////////////////////////////////
// 액터의 위치, 회전, 크기를 보관하는 컴포넌트다.
// 액터 생성 시 하나가 만들어진다. 로컬을 저장하고 월드 변환 조회 시 부모 계층을 합성한다.
////////////////////////////////////////////////////////////////////////////////
class FTransformComponent : public FActorComponent
{
public:
	/** 소속 액터를 지정하고 위치 0 / 회전 없음 / 크기 1배로 초기화한다. */
	explicit FTransformComponent(FActor& ownerActor);

	/** 저장된 로컬 위치 / 회전 / 크기를 묶은 트랜스폼의 복사본을 반환한다. */
	[[nodiscard]] FTransform GetTransform() const;

	/** 유한한 값 / 단위 회전의 로컬 변환을 복사해 저장한다. 검사·정규화·월드 좌표 변환은 수행하지 않는다. */
	void SetRelativeTransform(const FTransform& newTransform);

	/** 로컬 * 부모 월드를 계산해 값으로 반환한다. 루트는 로컬 그대로다. 캐시는 없으며 유한 값 / 단위 회전과 순환 없는 계층이 전제다. */
	[[nodiscard]] FTransform GetComponentTransform() const;

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
