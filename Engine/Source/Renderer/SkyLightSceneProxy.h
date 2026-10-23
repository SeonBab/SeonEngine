#pragma once

#include "Core/Math/Vector3.h"

class USkyLightComponent;

////////////////////////////////////////////////////////////////////////////////
// 세기가 반영된 균일 환경광 RGB를 복사해 보관한다. 컴포넌트 주소나 GPU 자원은 보관하지 않는다.
// 같은 스레드에서 생성·조회·갱신한다. Scene은 주소만 빌리고 Renderer를 통해 균일 주변광으로 전달한다.
////////////////////////////////////////////////////////////////////////////////
class FSkyLightSceneProxy
{
public:
	/** 컴포넌트의 선형 RGB에 세기를 곱해 복사한다. 입력 컴포넌트는 이 호출 동안만 빌린다. */
	explicit FSkyLightSceneProxy(const USkyLightComponent& component);

	// 이후 등록에 사용할 객체 주소와 정체성을 유지한다. 소유 포인터의 이동은 객체 이동과 다르다.
	FSkyLightSceneProxy(const FSkyLightSceneProxy&) = delete;
	FSkyLightSceneProxy& operator=(const FSkyLightSceneProxy&) = delete;
	FSkyLightSceneProxy(FSkyLightSceneProxy&&) = delete;
	FSkyLightSceneProxy& operator=(FSkyLightSceneProxy&&) = delete;

	/** 세기가 이미 반영된 선형 RGB 사본이다. 알파는 없고 RGB 0은 환경광 기여 없음이다. */
	FVector3 GetLightColor() const;

	/** 유한한 0 이상인 세기 반영 RGB를 저장한다. 세기를 다시 곱하거나 검사·GPU 업로드하지 않는다. */
	void SetLightColor(FVector3 newLightColor);

private:
	// 컴포넌트의 이후 변경을 자동 추적하지 않는 렌더값 사본이다.
	FVector3 lightColor{};
};
