#include "D3D11FontRenderData.h"

#include "Core/Logging/Log.h"
#include "Renderer/FontGlyph.h"
#include "Renderer/Texture2D.h"
#include "Renderer/TextureRenderData.h"
#include "Windows/WindowsHWrapper.h"

#include <directxtk/SpriteFont.h>

#include <cmath>

FD3D11FontRenderData::FD3D11FontRenderData() = default;

FD3D11FontRenderData::~FD3D11FontRenderData() = default;

bool FD3D11FontRenderData::GetLineSpacing(float& outLineSpacing) const
{
	if (!spriteFont)
	{
		WriteLog("Font", ELogLevel::Error, "Querying line spacing requires a prepared SpriteFont.");
		return false;
	}

	const float lineSpacing = spriteFont->GetLineSpacing();
	if (!std::isfinite(lineSpacing))
	{
		WriteLog("Font", ELogLevel::Error, "Font line spacing must be finite.");
		return false;
	}
	if (lineSpacing <= 0.0f)
	{
		WriteLog("Font", ELogLevel::Error, "Font line spacing must be positive ({}).", lineSpacing);
		return false;
	}

	outLineSpacing = lineSpacing;
	return true;
}

bool FD3D11FontRenderData::TryGetGlyph(uint32 codePoint, FFontGlyph& outGlyph) const
{
	if (codePoint > 0x10FFFF)
	{
		WriteLog("Font", ELogLevel::Error, "Glyph code point exceeds the Unicode range ({}).", codePoint);
		return false;
	}
	if (codePoint >= 0xD800 && codePoint <= 0xDFFF)
	{
		WriteLog("Font", ELogLevel::Error, "A surrogate is not a Unicode scalar value ({}).", codePoint);
		return false;
	}
	if (!spriteFont)
	{
		WriteLog("Font", ELogLevel::Error, "Querying a glyph requires a prepared SpriteFont.");
		return false;
	}
	if (!atlas)
	{
		WriteLog("Font", ELogLevel::Error, "Querying a glyph requires a font atlas.");
		return false;
	}
	const FTextureRenderData* const atlasData = atlas->GetRenderData();
	if (!atlasData)
	{
		WriteLog("Font", ELogLevel::Error, "Querying a glyph requires atlas render data.");
		return false;
	}
	const int32 atlasWidth = atlasData->GetWidth();
	const int32 atlasHeight = atlasData->GetHeight();
	if (atlasWidth <= 0)
	{
		WriteLog("Font", ELogLevel::Error, "Font atlas width must be positive ({}).", atlasWidth);
		return false;
	}
	if (atlasHeight <= 0)
	{
		WriteLog("Font", ELogLevel::Error, "Font atlas height must be positive ({}).", atlasHeight);
		return false;
	}

	// Windows SpriteFont는 16비트 wchar_t로 조회한다. 보조 평면 문자는 잘라내지 않고 대체 문자로 처리한다.
	wchar_t character = 0;
	bool hasGlyph = false;
	if (codePoint <= 0xFFFF)
	{
		character = static_cast<wchar_t>(codePoint);
		hasGlyph = spriteFont->ContainsCharacter(character);
	}
	if (!hasGlyph)
	{
		character = spriteFont->GetDefaultCharacter();
		if (character == 0)
		{
			WriteLog("Font", ELogLevel::Error, "Missing glyph {} has no configured fallback character.", codePoint);
			return false;
		}
		if (character >= 0xD800 && character <= 0xDFFF)
		{
			WriteLog("Font", ELogLevel::Error, "Font fallback character must be a Unicode scalar value.");
			return false;
		}
		if (!spriteFont->ContainsCharacter(character))
		{
			WriteLog("Font", ELogLevel::Error, "Font fallback character has no glyph.");
			return false;
		}
	}

	// FindGlyph는 대체 문자까지 없으면 예외를 던진다. 존재를 먼저 확인한 문자만 전달한다.
	const DirectX::SpriteFont::Glyph* const glyph = spriteFont->FindGlyph(character);
	if (!glyph)
	{
		WriteLog("Font", ELogLevel::Error, "SpriteFont returned no glyph for a verified character.");
		return false;
	}
	const RECT& rect = glyph->Subrect;
	if (rect.left < 0 || rect.top < 0)
	{
		WriteLog("Font", ELogLevel::Error, "Glyph atlas origin must be nonnegative.");
		return false;
	}
	if (rect.right < rect.left || rect.bottom < rect.top)
	{
		WriteLog("Font", ELogLevel::Error, "Glyph atlas bounds must not be inverted.");
		return false;
	}
	if (rect.right > atlasWidth || rect.bottom > atlasHeight)
	{
		WriteLog("Font", ELogLevel::Error, "Glyph rectangle exceeds the font atlas dimensions.");
		return false;
	}
	if (!std::isfinite(glyph->XOffset))
	{
		WriteLog("Font", ELogLevel::Error, "Glyph horizontal offset must be finite.");
		return false;
	}
	if (!std::isfinite(glyph->YOffset))
	{
		WriteLog("Font", ELogLevel::Error, "Glyph vertical offset must be finite.");
		return false;
	}
	if (!std::isfinite(glyph->XAdvance))
	{
		WriteLog("Font", ELogLevel::Error, "Glyph trailing advance must be finite.");
		return false;
	}

	FFontGlyph candidate;
	candidate.size = {static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top)};
	candidate.offset = {glyph->XOffset, glyph->YOffset};
	// SpriteFont의 XAdvance는 사각형 뒤의 이동만 뜻한다. 공통 값에는 왼쪽 오프셋과 폭도 포함한다.
	candidate.advanceX = glyph->XOffset + candidate.size.x + glyph->XAdvance;
	if (!std::isfinite(candidate.advanceX))
	{
		WriteLog("Font", ELogLevel::Error, "Glyph total horizontal advance must be finite.");
		return false;
	}
	const float width = static_cast<float>(atlasWidth);
	const float height = static_cast<float>(atlasHeight);
	candidate.uvMin = {static_cast<float>(rect.left) / width, static_cast<float>(rect.top) / height};
	candidate.uvMax = {static_cast<float>(rect.right) / width, static_cast<float>(rect.bottom) / height};
	candidate.atlas = atlas.get();
	outGlyph = candidate;
	return true;
}
