#pragma once

#include "Core/Math/Vector3.h"
#include "Core/Templates/SharedPtr.h"

class UTexture;

////////////////////////////////////////////////////////////////////////////////
// 재질의 렌더 조회 경계를 위한 공통 Proxy 타입 선언.
// 구체 Proxy가 색상 텍스처, 정반사와 알파 마스킹 설정 조회를 구현하며 같은 스레드에서 사용한다.
////////////////////////////////////////////////////////////////////////////////
class FMaterialRenderProxy
{
public:
	/** 기반 포인터로 소유한 구체 재질 Proxy의 소멸자까지 호출한다. */
	virtual ~FMaterialRenderProxy();

	/** 색상 텍스처의 공유 참조를 반환한다. 미지정이면 nullptr이며 반환 참조가 객체 수명을 유지한다. GPU 유효성은 검사하지 않는다. */
	virtual TSharedPtr<UTexture> GetBaseColorTexture() const = 0;

	/** 선형 RGB 정반사 반사율의 값 사본을 반환한다. 각 성분은 유한한 0~1이며 검정은 기여 없음이다. */
	virtual FVector3 GetSpecularColor() const = 0;

	/** Blinn-Phong 광택 지수의 값 사본을 반환한다. 유한한 1 이상이며 높을수록 하이라이트가 좁아진다. */
	virtual float GetShininess() const = 0;

	/** 같은 스레드에서 픽셀 제거 사용 여부를 조회한다. false이면 기존 불투명 그리기다. */
	virtual bool IsAlphaMasked() const = 0;

	/** 기준값과 같은 알파는 남긴다. 유한한 0~1 값 사본이며 마스킹이 꺼지면 사용하지 않는다. */
	virtual float GetAlphaCutoff() const = 0;
};
