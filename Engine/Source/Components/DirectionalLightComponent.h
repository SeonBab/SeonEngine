#pragma once

#include "ActorComponent.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Vector3.h"

/** 방향광의 색, 세기와 Scene 등록을 관리한다. 방향은 소속 액터의 앞 방향이며 GPU 전달은 아직 없다. */
class UDirectionalLightComponent : public UActorComponent
{
public:
	/** 소속 액터를 지정하고 흰색과 밝기 배율 1로 초기화한다. 생성만으로 액터에 등록하지 않는다. */
	explicit UDirectionalLightComponent(AActor& owner);

	/** 남은 Scene 등록을 해제한다. 소속 액터와 World의 Scene은 컴포넌트보다 오래 살아 있어야 한다. */
	~UDirectionalLightComponent() override;

	/**
	 * 소속 액터의 앞 방향을 빛이 진행하는 월드 방향으로 반환한다. 표면에서 광원을 향하는 방향은 이 값의 반대다.
	 * 부모 회전, 입력 전제, 합성의 표현 한계는 UTransformComponent::GetForwardVector를 따른다.
	 * 방향은 별도 저장하지 않으며 액터의 현재 Transform에서 조회한다.
	 */
	[[nodiscard]] FVector3 GetLightDirection() const;

	/** 선형 RGB 색의 복사본을 반환한다. x, y, z는 각각 R, G, B이며 알파는 없다. */
	FVector3 GetLightColor() const;

	/** 각 성분이 유한한 0 이상인 선형 RGB 색을 저장한다. 1 초과도 허용하며 검사, 보정, sRGB 변환은 하지 않는다. */
	void SetLightColor(const FVector3& newLightColor);

	/** 색에 곱할 밝기 배율을 반환한다. 0은 빛 없음, 1은 기본 밝기이며 lux 단위가 아니다. */
	float GetIntensity() const;

	/** 유한한 0 이상의 밝기 배율을 저장한다. 값 검사나 보정은 하지 않으며 화면 반영을 요청하지 않는다. */
	void SetIntensity(float newIntensity);

	/** 현재 방향, 색, 세기를 소속 World의 Scene에 등록한다. 중복 생성이나 Scene 등록 실패는 false이며 기존 상태를 유지한다. GPU 자원은 만들지 않는다. */
	[[nodiscard]] bool CreateRenderState();

	/**
	 * 현재 방향, 색, 세기를 등록된 Scene 항목에 복사한다. 변경 여부와 관계없이 호출할 때마다 전달한다.
	 * 미등록이거나 Scene에 보관한 ID가 없으면 false다. 등록을 복구하거나 ID를 변경하지 않는다.
	 * 자동 호출이나 GPU 전달은 수행하지 않는다.
	 */
	[[nodiscard]] bool UpdateRenderState();

	/** 보관한 ID의 Scene 등록 해제를 요청하고 ID를 0으로 초기화한다. 미등록이면 아무것도 하지 않으며 반복 호출할 수 있다. */
	void DestroyRenderState();

	/** Scene 등록 ID를 보관하는지 반환한다. 실제 Scene 항목의 존재나 GPU 준비 상태는 검사하지 않는다. */
	[[nodiscard]] bool IsRenderStateCreated() const;

private:
	FVector3 lightColor{1.0f, 1.0f, 1.0f};
	float intensity = 1.0f;
	uint64 sceneLightId = 0;
};
