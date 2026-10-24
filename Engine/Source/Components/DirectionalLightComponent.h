#pragma once

#include "Core/Math/Vector3.h"
#include "Core/Templates/UniquePtr.h"
#include "RenderStateComponent.h"

class FDirectionalLightSceneProxy;

////////////////////////////////////////////////////////////////////////////////
// 방향광의 색과 세기를 보관하고 렌더용 Proxy를 소유한다.
// 방향은 소속 액터의 앞 방향이며 Scene에는 Proxy 주소를 등록한다.
////////////////////////////////////////////////////////////////////////////////
class UDirectionalLightComponent : public URenderStateComponent
{
#pragma region Lifecycle

public:
	/** 소속 액터를 지정하고 흰색과 밝기 배율 1로 초기화한다. 생성만으로 액터에 등록하지 않는다. */
	explicit UDirectionalLightComponent(AActor& owner);

	/** 남은 Scene 등록을 해제한다. 소속 액터와 World의 Scene은 컴포넌트보다 오래 살아 있어야 한다. */
	~UDirectionalLightComponent() override;

#pragma endregion

#pragma region Light

public:
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

private:
	FVector3 lightColor{1.0f, 1.0f, 1.0f};
	float intensity = 1.0f;

#pragma endregion

#pragma region RenderState

public:
	/**
	 * 현재 설정으로 Proxy를 생성하고 Scene에 주소를 등록한다.
	 * 이미 생성됐거나 등록에 실패하면 false이며 기존 상태를 유지한다.
	 * Proxy는 컴포넌트가 소유하며 GPU 자원은 만들지 않는다.
	 */
	[[nodiscard]] bool CreateRenderState() override;

	/**
	 * Scene에 등록된 소유 Proxy에 현재 방향과 세기가 반영된 색을 전달한다.
	 * 미생성이거나 Scene 등록이 일치하지 않으면 false다.
	 * 같은 스레드에서 사용하며 재등록이나 GPU 업로드는 수행하지 않는다.
	 */
	[[nodiscard]] bool SendRenderUpdates() override;

	/** Scene 등록을 먼저 해제한 뒤 소유 Proxy를 파괴한다. 미생성이면 아무것도 하지 않으며 반복 호출할 수 있다. */
	void DestroyRenderState() override;

	/** 소유 Proxy가 있는지 반환한다. 외부에서 Scene 등록을 해제했는지나 GPU 준비 상태는 검사하지 않는다. */
	[[nodiscard]] bool IsRenderStateCreated() const override;

private:
	// 이 컴포넌트가 소유한다. Scene 등록을 해제한 뒤 파괴하며 nullptr는 렌더 상태 미생성이다.
	TUniquePtr<FDirectionalLightSceneProxy> sceneProxy{};

#pragma endregion
};
