#pragma once

#include "Core/Math/Vector3.h"

class UDirectionalLightComponent;

////////////////////////////////////////////////////////////////////////////////
// 방향광의 월드 진행 방향과 세기가 반영된 선형 RGB를 렌더 상태로 보관한다.
// 컴포넌트가 소유하고 Scene은 주소를 빌린다. 컴포넌트 주소나 GPU 자원은 보관하지 않는다.
// 생성, 갱신, 조회와 파괴는 같은 스레드에서 수행한다.
////////////////////////////////////////////////////////////////////////////////
class FDirectionalLightSceneProxy
{
public:
	/** 컴포넌트의 현재 방향과 색, 세기를 읽는다. 생성만으로 Scene에 등록하지 않는다. */
	explicit FDirectionalLightSceneProxy(const UDirectionalLightComponent& component);

	/** 빛이 진행하는 월드 공간의 단위 방향을 반환한다. 표면에서 광원을 향하는 방향은 반대다. */
	FVector3 GetLightDirection() const;

	/** 세기가 이미 반영된 선형 RGB의 복사본을 반환한다. */
	FVector3 GetLightColor() const;

	/**
	 * 빛이 진행하는 월드 공간의 단위 방향을 저장한다.
	 * 입력 검사나 정규화, GPU 업로드는 수행하지 않는다.
	 */
	void SetLightDirection(FVector3 newLightDirection);

	/**
	 * 세기가 이미 반영된, 유한한 0 이상 성분의 선형 RGB를 저장한다.
	 * 세기를 다시 곱하거나 입력 보정, GPU 업로드를 수행하지 않는다.
	 */
	void SetLightColor(FVector3 newLightColor);

private:
	// 빛이 진행하는 월드 공간의 단위 방향. 컴포넌트의 방향 계약을 따른다.
	FVector3 lightDirection;

	// 선형 RGB 색과 밝기 배율의 곱. 사용하는 쪽에서 세기를 다시 곱하지 않는다.
	FVector3 lightColor;
};
