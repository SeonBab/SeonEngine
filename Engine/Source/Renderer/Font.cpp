#include "Font.h"

#include "Core/Logging/Log.h"
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

bool UFont::GetLineSpacing(float& outLineSpacing) const
{
	if (!renderData)
	{
		WriteLog("Font", ELogLevel::Error, "Querying line spacing requires font render data.");
		return false;
	}

	return renderData->GetLineSpacing(outLineSpacing);
}

bool UFont::TryGetGlyph(uint32 codePoint, FFontGlyph& outGlyph) const
{
	if (!renderData)
	{
		WriteLog("Font", ELogLevel::Error, "Querying a glyph requires font render data.");
		return false;
	}

	return renderData->TryGetGlyph(codePoint, outGlyph);
}
