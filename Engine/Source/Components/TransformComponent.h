#pragma once

#include "ActorComponent.h"
#include "Core/Math/Quat.h"
#include "Core/Math/Rotator.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector3.h"

////////////////////////////////////////////////////////////////////////////////
// 액터에 붙어 위치, 회전, 크기 데이터를 보관한다.
// 부모 기준 상대값을 저장하고 값이나 부모가 바뀌면 자신과 자손의 월드 변환을 즉시 갱신한다.
// 모든 변경과 조회는 같은 스레드에서 수행한다. GPU 반영은 기존 렌더 상태 갱신 경로가 맡는다.
////////////////////////////////////////////////////////////////////////////////
class UTransformComponent : public UActorComponent
{
#pragma region Lifecycle

public:
	/** 부모 없는 소속 액터에 상대값과 월드 변환을 항등으로 초기화한다. 자동 등록은 하지 않는다. */
	explicit UTransformComponent(AActor& owner);

#pragma endregion

#pragma region RelativeTransform

public:
	/** 상대 세 값으로 로컬 변환을 구성해 반환한다. 회전은 Quat으로 변환하며 루트에서는 월드 변환과 같다. */
	FTransform GetTransform() const;

	/** 유한한 값과 정규화된 회전을 요구한다. 회전을 대표 각도로 바꿔 상대값을 저장한 뒤 자손까지 월드 변환을 갱신한다. 원래 Euler 표현이나 Quat 성분 자체는 보존하지 않는다. */
	void SetTransform(const FTransform& newTransform);

	/** 저장된 위치의 복사본을 반환한다. */
	FVector3 GetPosition() const;

	/** 상대 위치를 저장하고 자손까지 월드 변환을 갱신한다. 값 검사는 수행하지 않는다. */
	void SetPosition(FVector3 newPosition);

	/** 저장된 상대 각도를 정규화된 회전 쿼터니언으로 변환해 반환한다. */
	FQuat GetQuaternion() const;

	/** 유한하고 정규화된 회전을 대표 상대 각도로 저장하고 자손까지 월드 변환을 갱신한다. 원래 Euler 표현이나 Quat 성분 자체는 보존하지 않는다. */
	void SetQuaternion(FQuat newRotation);

	/** 저장된 부모 기준 도 단위 각도를 그대로 반환한다. 정규화나 대표 각도 변환은 하지 않는다. */
	FRotator GetRotation() const;

	/** 유한한 상대 각도를 그대로 저장하고 자손까지 월드 변환을 갱신한다. 캐시 계산과 Transform/Quaternion 조회에는 Quat 변환을 사용한다. */
	void SetRotation(FRotator newRotation);

	/** 저장된 축별 크기 배율의 복사본을 반환한다. */
	FVector3 GetScale() const;

	/** 상대 크기 배율을 저장하고 자손까지 월드 변환을 갱신한다. 값 검사는 수행하지 않는다. */
	void SetScale(FVector3 newScale);

private:
	FVector3 relativePosition{};
	FRotator relativeRotation{}; // 각도 API로 설정한 표현을 유지한다. Quat/Transform 설정 시에는 대표 각도로 대체된다.
	FVector3 relativeScale{1.0f, 1.0f, 1.0f};

#pragma endregion

#pragma region WorldTransform

public:
	/** 보관한 월드 변환의 사본을 반환한다. 조회 시 재계산하지 않는다. */
	[[nodiscard]] FTransform GetComponentTransform() const;

	/** 상대값과 부모의 현재 월드 변환으로 캐시를 갱신한 뒤 자손에 전파한다. setter와 World의 관계 변경 경로가 호출하며 렌더/GPU 갱신은 수행하지 않는다. */
	void UpdateComponentToWorld();

	/**
	 * 부모를 반영한 월드 회전으로 로컬 +Z축을 돌린 앞 방향을 반환한다.
	 * 유한한 Transform과 정규화된 회전이 전제다. 최종 월드 Transform의 위치, 크기는 방향에 적용하지 않는다.
	 * 저장 값을 변경하지 않으며, 부모 합성의 표현 한계는 GetComponentTransform을 따른다.
	 */
	[[nodiscard]] FVector3 GetForwardVector() const;

	/** 월드 변환을 상대값으로 계산해 저장한 뒤 자손까지 갱신한다. 표현 한계와 대체 계산은 FTransform::GetRelativeTransform을 따른다. */
	void SetComponentTransform(const FTransform& newTransform);

private:
	FTransform componentToWorld{}; // 상대값과 부모 계층으로 계산한 CPU 월드 변환이다. 변경 시 갱신하며 최초 부모 없는 상태는 항등이다.

#pragma endregion
};
