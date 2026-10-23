#pragma once

#include "ActorComponent.h"
#include "Core/Math/Vector3.h"
#include "Core/Templates/UniquePtr.h"

class FSkyLightSceneProxy;

////////////////////////////////////////////////////////////////////////////////
// 균일 환경광의 선형 RGB와 밝기 배율을 보관한다. 위치나 회전은 조명값에 영향을 주지 않는다.
// Proxy를 소유하고 Scene은 주소를 빌린다. GPU 전달은 Scene/Renderer가 맡고 환경 캡처는 지원하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class USkyLightComponent : public UActorComponent
{
public:
	/** 소속 액터와 흰색·세기 1을 준비한다. 생성만으로 액터에 추가하거나 렌더 상태를 만들지 않는다. */
	explicit USkyLightComponent(AActor& owner);

	/** Scene 등록을 먼저 해제하고 소유 Proxy를 파괴한다. Actor와 World는 그때까지 살아 있어야 한다. */
	~USkyLightComponent() override;

	/** 선형 RGB 사본이다. x/y/z가 R/G/B이며 알파는 없다. */
	FVector3 GetLightColor() const;

	/** 유한한 0 이상 선형 RGB를 저장한다. 검사/clamp/sRGB 변환이나 Proxy 갱신은 하지 않는다. */
	void SetLightColor(const FVector3& newLightColor);

	/** 색에 곱할 무차원 밝기 배율이다. 0은 기여 없음, 1은 기본 밝기이며 lux 단위가 아니다. */
	float GetIntensity() const;

	/** 유한한 0 이상 배율을 저장한다. 1 초과도 허용하며 검사·보정·Proxy 갱신은 하지 않는다. */
	void SetIntensity(float newIntensity);

	/** 현재 설정으로 Proxy를 만들고 Scene에 등록한다. 중복 생성은 false다. GPU 자원은 만들지 않는다. */
	[[nodiscard]] bool CreateRenderState();

	/** 등록된 소유 Proxy에 색×세기를 전달한다. 미생성/외부 등록 해제는 false이며 활성 순서를 바꾸지 않는다. */
	[[nodiscard]] bool UpdateRenderState();

	/** Scene에서 주소를 제거한 뒤 Proxy를 파괴한다. 같은 스레드에서 호출하며 반복할 수 있다. */
	void DestroyRenderState();

	/** 소유 Proxy가 있는지 조회한다. Scene 등록 여부나 현재 활성 여부를 검사하지 않는다. */
	[[nodiscard]] bool IsRenderStateCreated() const;

private:
	FVector3 lightColor{1.0f, 1.0f, 1.0f};
	float intensity = 1.0f;

	// 단독 소유한다. Scene 등록을 해제한 뒤 파괴하며 nullptr는 렌더 상태 미생성이다.
	TUniquePtr<FSkyLightSceneProxy> sceneProxy{};
};
