#include "Material.h"

#include "MaterialRenderProxy.h"

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
	TSharedPtr<UTexture2D> GetBaseColorTexture() const override { return material->GetBaseColorTexture(); }

private:
	// 비소유 주소다. 재질을 파괴하거나 이동한 뒤에는 조회할 수 없다.
	UMaterial* material;
};

UMaterial::UMaterial()
	: renderProxy(MakeUnique<FDefaultMaterialInstance>(this))
{
}

UMaterial::~UMaterial() = default;

void UMaterial::SetBaseColorTexture(TSharedPtr<UTexture2D> texture)
{
	baseColorTexture = std::move(texture);
}

TSharedPtr<UTexture2D> UMaterial::GetBaseColorTexture() const
{
	return baseColorTexture;
}

FMaterialRenderProxy* UMaterial::GetRenderProxy() const
{
	return renderProxy.get();
}
