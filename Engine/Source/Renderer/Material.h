#pragma once

#include "Core/Templates/SharedPtr.h"
#include "Core/Templates/UniquePtr.h"
#include "MaterialInterface.h"

class FMaterialRenderProxy;
class UTexture2D;

////////////////////////////////////////////////////////////////////////////////
// 고정 셰이더에서 사용할 색상 텍스처를 공유 참조하는 구체 재질이다.
// 설정과 조회는 같은 스레드에서 사용하며 GPU 생성이나 그리기를 수행하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class UMaterial : public UMaterialInterface
{
#pragma region Lifecycle

public:
	/** 현재 스레드에서 이 재질을 조회하는 전용 Proxy를 함께 생성한다. GPU 자원은 생성하지 않는다. */
	UMaterial();

	/** 전용 Proxy를 먼저 파괴한 뒤 텍스처 공유 참조를 해제한다. */
	~UMaterial() override;

	// 전용 Proxy가 이 객체의 주소를 빌리므로 생성 후 주소와 객체 정체성을 유지한다.
	UMaterial(const UMaterial&) = delete;

	UMaterial& operator=(const UMaterial&) = delete;

	UMaterial(UMaterial&&) = delete;

	UMaterial& operator=(UMaterial&&) = delete;

	/**
	 * 같은 스레드에서 사용하는, 텍스처 미지정 기본 재질의 비소유 주소를 반환한다.
	 * 최초 호출 때 생성하고 이후 같은 객체를 반환한다. 호출자는 삭제하거나 소유권을 가져오거나 설정을 변경하지 않는다.
	 * 이 코드를 포함한 모듈의 정적 객체 정리 전까지만 사용한다. GPU 생성이나 Scene 등록은 하지 않는다.
	 */
	static UMaterial* GetDefaultMaterial();

#pragma endregion

#pragma region BaseColor

public:
	/** 색상 텍스처의 공유 참조를 교체한다. nullptr는 미지정이며 GPU 검사나 렌더 상태 갱신은 하지 않는다. */
	void SetBaseColorTexture(TSharedPtr<UTexture2D> texture);

	/** 공유 참조의 사본을 반환한다. 미지정이면 nullptr이며 사본을 유지하는 동안 텍스처 객체가 살아 있다. */
	TSharedPtr<UTexture2D> GetBaseColorTexture() const;

private:
	// 텍스처 객체의 수명을 공유한다. 기본 nullptr는 색상 텍스처 미지정 상태다.
	TSharedPtr<UTexture2D> baseColorTexture;

#pragma endregion

#pragma region RenderProxy

public:
	/** 생성 시 만든 전용 Proxy의 비소유 주소를 반환한다. 같은 스레드에서 재질 수명 안에 사용하며 삭제하지 않는다. */
	FMaterialRenderProxy* GetRenderProxy() const override;

private:
	// 단독 소유한다. 선언의 역순으로 파괴돼 텍스처 참조보다 먼저 정리된다.
	TUniquePtr<FMaterialRenderProxy> renderProxy;

#pragma endregion
};
