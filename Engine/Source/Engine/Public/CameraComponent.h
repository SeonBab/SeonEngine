#pragma once

#include "ActorComponent.h"

class FActor;
struct FMinimalViewInfo;

////////////////////////////////////////////////////////////////////////////////
// 액터의 카메라 설정을 보관한다. 위치와 회전은 소속 액터의 트랜스폼을 사용한다.
// 월드 위치 / 회전과 수평 시야각을 조회하며 카메라 선택과 렌더 전달은 아직 연결하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FCameraComponent : public FActorComponent
{
public:
	/** 소속 액터와 기본 수평 시야각 90도를 준비한다. 생성만으로 액터에 추가되지 않는다. */
	explicit FCameraComponent(FActor& ownerActor);

	/** 수평 시야각을 도 단위로 저장한다. 값 검사나 투영 행렬 갱신은 수행하지 않는다. */
	void SetFieldOfView(float inFieldOfView);

	/** 저장한 수평 시야각을 도 단위의 값으로 반환한다. */
	float GetFieldOfView() const;

	/** 소속 액터의 월드 위치 / 회전과 수평 시야각을 출력 값에 채운다. 필수 트랜스폼과 유한 값 / 단위 회전이 전제다. */
	void GetCameraView(FMinimalViewInfo& outView) const;

private:
	float fieldOfView = 90.0f;
};
