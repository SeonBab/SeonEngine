#include "Font.h"

#include "Renderer/FontRenderData.h"

#include <utility>

UFont::UFont() = default;

UFont::~UFont() = default;

const FFontRenderData* UFont::GetRenderData() const
{
	return renderData.get();
}

void UFont::SetRenderData(TUniquePtr<FFontRenderData>&& newRenderData)
{
	renderData = std::move(newRenderData);
}
