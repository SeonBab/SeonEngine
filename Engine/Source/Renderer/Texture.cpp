#include "Texture.h"

#include "Renderer/TextureRenderData.h"

#include <utility>

UTexture::UTexture() = default;

UTexture::~UTexture() = default;

UTexture::UTexture(UTexture&&) noexcept = default;

UTexture& UTexture::operator=(UTexture&&) noexcept = default;

const FTextureRenderData* UTexture::GetRenderData() const
{
	return renderData.get();
}

void UTexture::SetRenderData(TUniquePtr<FTextureRenderData>&& newRenderData)
{
	renderData = std::move(newRenderData);
}
