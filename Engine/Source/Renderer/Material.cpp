#include "Material.h"

#include "MaterialRenderProxy.h"

#include <algorithm>
#include <cmath>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
// UMaterial 전용 렌더 조회 구현이다. 설정을 복사하지 않고 소유 재질에 조회를 위임한다.
// 재질 주소를 빌리며 같은 스레드에서 사용한다.
////////////////////////////////////////////////////////////////////////////////
class FDefaultMaterialInstance : public FMaterialRenderProxy
{
public:
	/** nullptr가 아닌 재질 주소를 빌린다. 재질은 이 Proxy의 모든 조회가 끝날 때까지 살아 있어야 한다. */
	explicit FDefaultMaterialInstance(UMaterial* inMaterial)
		: material(inMaterial)
	{
	}

	/** 같은 스레드에서 재질의 현재 색상 텍스처 공유 참조를 얻는다. GPU 유효성은 검사하지 않는다. */
	TSharedPtr<UTexture> GetBaseColorTexture() const override { return material->GetBaseColorTexture(); }

	/** 소유 재질의 현재 정반사 색·세기를 값으로 조회한다. GPU 갱신은 하지 않는다. */
	FVector3 GetSpecularColor() const override { return material->GetSpecularColor(); }

	/** 소유 재질의 현재 광택 지수를 값으로 조회한다. GPU 갱신은 하지 않는다. */
	float GetShininess() const override { return material->GetShininess(); }

	bool IsAlphaMasked() const override { return material->IsAlphaMasked(); }

	float GetAlphaCutoff() const override { return material->GetAlphaCutoff(); }

private:
	// 비소유 주소다. 재질을 파괴하거나 이동한 뒤에는 조회할 수 없다.
	UMaterial* material;
};

#pragma region Lifecycle

UMaterial::UMaterial()
	: renderProxy(MakeUnique<FDefaultMaterialInstance>(this))
{
}

UMaterial::~UMaterial() = default;

UMaterial* UMaterial::GetDefaultMaterial()
{
	// 최초 조회 때 재질과 전용 Proxy를 만들며 모듈의 정적 객체 정리 때 함께 파괴한다.
	static UMaterial defaultMaterial;

	return &defaultMaterial;
}

#pragma endregion

#pragma region BaseColor

void UMaterial::SetBaseColorTexture(TSharedPtr<UTexture> texture)
{
	baseColorTexture = std::move(texture);
}

TSharedPtr<UTexture> UMaterial::GetBaseColorTexture() const
{
	return baseColorTexture;
}

#pragma endregion

#pragma region Specular

void UMaterial::SetSpecularColor(FVector3 color)
{
	if (!std::isfinite(color.x) || !std::isfinite(color.y) || !std::isfinite(color.z)) { return; }

	specularColor = {
		std::clamp(color.x, 0.0f, 1.0f),
		std::clamp(color.y, 0.0f, 1.0f),
		std::clamp(color.z, 0.0f, 1.0f)};
}

FVector3 UMaterial::GetSpecularColor() const
{
	return specularColor;
}

void UMaterial::SetShininess(float newShininess)
{
	if (!std::isfinite(newShininess)) { return; }

	shininess = std::max(newShininess, 1.0f);
}

float UMaterial::GetShininess() const
{
	return shininess;
}

#pragma endregion

#pragma region AlphaMask

void UMaterial::SetAlphaMasked(bool bNewAlphaMasked)
{
	bAlphaMasked = bNewAlphaMasked;
}

bool UMaterial::IsAlphaMasked() const
{
	return bAlphaMasked;
}

void UMaterial::SetAlphaCutoff(float newAlphaCutoff)
{
	if (!std::isfinite(newAlphaCutoff)) { return; }

	alphaCutoff = std::clamp(newAlphaCutoff, 0.0f, 1.0f);
}

float UMaterial::GetAlphaCutoff() const
{
	return alphaCutoff;
}

#pragma endregion

#pragma region RenderProxy

FMaterialRenderProxy* UMaterial::GetRenderProxy() const
{
	return renderProxy.get();
}

#pragma endregion
