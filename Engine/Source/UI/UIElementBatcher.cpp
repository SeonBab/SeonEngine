#include "UIElementBatcher.h"

#include "Core/Containers/SeonString.h"
#include "Core/CoreTypes.h"
#include "Core/Logging/Log.h"
#include "Renderer/Font.h"
#include "Renderer/FontGlyph.h"
#include "Renderer/Texture.h"
#include "Renderer/TextureRenderData.h"
#include "UI/UIBatchData.h"
#include "UI/UIDrawElementList.h"
#include "UI/UIImageDrawElement.h"
#include "UI/UITextDrawElement.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
	bool IsFinite(const FVector2& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y);
	}

	// 호출부가 미해석 바이트의 존재를 보장한다. 성공할 때만 위치와 코드 포인트를 갱신한다.
	bool ReadCodePoint(FStringView text, size_t& cursor, uint32& outCodePoint)
	{
		const uint8 first = static_cast<uint8>(text[cursor]);
		uint32 value = 0;
		uint32 minimum = 0;
		size_t byteCount = 0;
		if (first <= 0x7F)
		{
			value = first;
			byteCount = 1;
		}
		else if (first >= 0xC2 && first <= 0xDF)
		{
			value = first & 0x1F;
			minimum = 0x80;
			byteCount = 2;
		}
		else if (first >= 0xE0 && first <= 0xEF)
		{
			value = first & 0x0F;
			minimum = 0x800;
			byteCount = 3;
		}
		else if (first >= 0xF0 && first <= 0xF4)
		{
			value = first & 0x07;
			minimum = 0x10000;
			byteCount = 4;
		}
		else
		{
			WriteLog("UI", ELogLevel::Error, "Invalid UTF-8 leading byte at offset {}.", cursor);
			return false;
		}

		if (byteCount > text.size() - cursor)
		{
			WriteLog("UI", ELogLevel::Error, "Truncated UTF-8 sequence at offset {}.", cursor);
			return false;
		}
		for (size_t i = 1; i < byteCount; ++i)
		{
			const uint8 continuation = static_cast<uint8>(text[cursor + i]);
			if ((continuation & 0xC0) != 0x80)
			{
				WriteLog("UI", ELogLevel::Error, "Invalid UTF-8 continuation byte at offset {}.", cursor + i);
				return false;
			}
			value = (value << 6) | (continuation & 0x3F);
		}
		if (value < minimum)
		{
			WriteLog("UI", ELogLevel::Error, "Overlong UTF-8 sequence at offset {}.", cursor);
			return false;
		}
		if (value > 0x10FFFF)
		{
			WriteLog("UI", ELogLevel::Error, "UTF-8 code point exceeds the Unicode range at offset {}.", cursor);
			return false;
		}
		if (value >= 0xD800 && value <= 0xDFFF)
		{
			WriteLog("UI", ELogLevel::Error, "UTF-8 must not encode a surrogate at offset {}.", cursor);
			return false;
		}
		cursor += byteCount;
		outCodePoint = value;

		return true;
	}

	bool ValidateTexture(const UTexture* texture)
	{
		if (!texture)
		{
			WriteLog("UI", ELogLevel::Error, "UI geometry requires a texture.");
			return false;
		}
		const FTextureRenderData* const textureData = texture->GetRenderData();
		if (!textureData)
		{
			WriteLog("UI", ELogLevel::Error, "UI geometry requires texture render data.");
			return false;
		}
		if (textureData->GetWidth() <= 0)
		{
			WriteLog("UI", ELogLevel::Error, "UI texture width must be positive.");
			return false;
		}
		if (textureData->GetHeight() <= 0)
		{
			WriteLog("UI", ELogLevel::Error, "UI texture height must be positive.");
			return false;
		}
		const ETextureAlphaRepresentation alpha = textureData->GetAlphaRepresentation();
		if (alpha != ETextureAlphaRepresentation::Straight && alpha != ETextureAlphaRepresentation::Premultiplied)
		{
			WriteLog("UI", ELogLevel::Error, "UI texture uses an unsupported alpha representation.");
			return false;
		}

		return true;
	}

	bool ValidateGlyph(const FFontGlyph& glyph)
	{
		if (!ValidateTexture(glyph.atlas)) { return false; }
		if (!IsFinite(glyph.size) || glyph.size.x < 0.0f || glyph.size.y < 0.0f)
		{
			WriteLog("UI", ELogLevel::Error, "Glyph size must be finite and nonnegative.");
			return false;
		}
		if (!IsFinite(glyph.offset))
		{
			WriteLog("UI", ELogLevel::Error, "Glyph offset must be finite.");
			return false;
		}
		if (!std::isfinite(glyph.advanceX))
		{
			WriteLog("UI", ELogLevel::Error, "Glyph advance must be finite.");
			return false;
		}
		if (!IsFinite(glyph.uvMin) || !IsFinite(glyph.uvMax))
		{
			WriteLog("UI", ELogLevel::Error, "Glyph UV coordinates must be finite.");
			return false;
		}
		if (glyph.uvMin.x < 0.0f || glyph.uvMin.y < 0.0f)
		{
			WriteLog("UI", ELogLevel::Error, "Glyph minimum UV must be nonnegative.");
			return false;
		}
		if (glyph.uvMax.x > 1.0f || glyph.uvMax.y > 1.0f)
		{
			WriteLog("UI", ELogLevel::Error, "Glyph maximum UV must not exceed one.");
			return false;
		}
		if (glyph.uvMax.x < glyph.uvMin.x || glyph.uvMax.y < glyph.uvMin.y)
		{
			WriteLog("UI", ELogLevel::Error, "Glyph UV bounds must not be inverted.");
			return false;
		}

		return true;
	}

	// 호출부에서 텍스처/UV/색상을 확인했다. 생성 좌표와 배열/GPU 바이트 범위는 여기에서 검사한다.
	bool AppendQuad(const UTexture* texture, const FVector2& position, const FVector2& size,
		const FVector2& uvMin, const FVector2& uvMax, const float (&color)[4], EUISampling sampling, FUIBatchData& data)
	{
		if (!IsFinite(position))
		{
			WriteLog("UI", ELogLevel::Error, "UI quad position must be finite.");
			return false;
		}
		if (!IsFinite(size) || size.x <= 0.0f || size.y <= 0.0f)
		{
			WriteLog("UI", ELogLevel::Error, "UI quad size must be finite and positive.");
			return false;
		}
		const float right = position.x + size.x;
		const float bottom = position.y + size.y;
		if (!std::isfinite(right) || !std::isfinite(bottom))
		{
			WriteLog("UI", ELogLevel::Error, "UI quad corner coordinates overflowed.");
			return false;
		}

		// 32비트 바이트 크기에 들어오는 공통 geometry만 만든다. 곱셈 전에 원소 개수로 상한을 검사한다.
		const size_t maxBytes = std::numeric_limits<uint32>::max();
		const size_t maxVertices = std::min(data.vertices.max_size(), maxBytes / sizeof(FUIVertex));
		const size_t maxIndices = std::min(data.indices.max_size(), maxBytes / sizeof(uint32));
		if (maxVertices < 4 || data.vertices.size() > maxVertices - 4)
		{
			WriteLog("UI", ELogLevel::Error, "UI vertices exceed the supported array or byte range.");
			return false;
		}
		if (maxIndices < 6 || data.indices.size() > maxIndices - 6)
		{
			WriteLog("UI", ELogLevel::Error, "UI indices exceed the supported array or byte range.");
			return false;
		}
		const bool merge = !data.renderBatches.empty() && data.renderBatches.back().texture == texture &&
			data.renderBatches.back().sampling == sampling;
		if (!merge && data.renderBatches.size() == data.renderBatches.max_size())
		{
			WriteLog("UI", ELogLevel::Error, "UI render batches exceed the supported array range.");
			return false;
		}

		const uint32 firstVertex = static_cast<uint32>(data.vertices.size());
		const uint32 firstIndex = static_cast<uint32>(data.indices.size());
		data.vertices.push_back({position, uvMin, {color[0], color[1], color[2], color[3]}});
		data.vertices.push_back({{right, position.y}, {uvMax.x, uvMin.y}, {color[0], color[1], color[2], color[3]}});
		data.vertices.push_back({{right, bottom}, uvMax, {color[0], color[1], color[2], color[3]}});
		data.vertices.push_back({{position.x, bottom}, {uvMin.x, uvMax.y}, {color[0], color[1], color[2], color[3]}});
		data.indices.insert(data.indices.end(), {firstVertex, firstVertex + 1, firstVertex + 2, firstVertex, firstVertex + 2, firstVertex + 3});
		if (merge)
		{
			data.renderBatches.back().indexCount += 6;
		}
		else
		{
			data.renderBatches.push_back({texture, firstIndex, 6, sampling});
		}

		return true;
	}
}

bool FUIElementBatcher::Build(const FUIDrawElementList& elements, FUIBatchData& outData) const
{
	FUIBatchData candidate;
	for (const FUIDrawElement& element : elements.GetElements())
	{
		if (const FUIImageDrawElement* image = std::get_if<FUIImageDrawElement>(&element))
		{
			if (!AddImageGeometry(*image, candidate)) { return false; }
		}
		else
		{
			if (!AddTextGeometry(std::get<FUITextDrawElement>(element), candidate)) { return false; }
		}
	}

	// 뒤쪽 글자나 요소가 실패해도 이전 출력 전체를 보존한다.
	outData = std::move(candidate);

	return true;
}

bool FUIElementBatcher::AddImageGeometry(const FUIImageDrawElement& element, FUIBatchData& data) const
{
	if (!ValidateTexture(element.texture)) { return false; }
	const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	return AppendQuad(element.texture, element.position, element.size, {0.0f, 0.0f}, {1.0f, 1.0f}, white, EUISampling::PointClamp, data);
}

bool FUIElementBatcher::AddTextGeometry(const FUITextDrawElement& element, FUIBatchData& data) const
{
	if (!element.font)
	{
		WriteLog("UI", ELogLevel::Error, "UI text requires a font.");
		return false;
	}
	if (!IsFinite(element.position))
	{
		WriteLog("UI", ELogLevel::Error, "UI text position must be finite.");
		return false;
	}
	if (!std::isfinite(element.scale) || element.scale <= 0.0f)
	{
		WriteLog("UI", ELogLevel::Error, "UI text scale must be finite and positive.");
		return false;
	}
	for (float component : element.color)
	{
		if (!std::isfinite(component) || component < 0.0f || component > 1.0f)
		{
			WriteLog("UI", ELogLevel::Error, "UI text color components must be finite and within zero to one.");
			return false;
		}
	}
	float lineSpacing = 0.0f;
	if (!element.font->GetLineSpacing(lineSpacing)) { return false; }
	const float lineAdvance = lineSpacing * element.scale;
	if (!std::isfinite(lineAdvance) || lineAdvance <= 0.0f)
	{
		WriteLog("UI", ELogLevel::Error, "Scaled text line spacing must be finite and positive.");
		return false;
	}

	// 첫 줄 기준점에 대한 상대 픽셀 좌표다. 행 시작은 0이며 음수 bearing을 제한하지 않는다.
	FVector2 pen{};
	size_t cursor = 0;
	while (cursor < element.text.size())
	{
		uint32 codePoint = 0;
		if (!ReadCodePoint(element.text, cursor, codePoint)) { return false; }
		if (codePoint == 0)
		{
			WriteLog("UI", ELogLevel::Error, "UI text must not contain NUL.");
			return false;
		}
		if (codePoint == '\t')
		{
			WriteLog("UI", ELogLevel::Error, "UI text tab expansion is not supported.");
			return false;
		}
		if (codePoint == '\r') { continue; }
		if (codePoint == '\n')
		{
			pen.x = 0.0f;
			pen.y += lineAdvance;
			if (!std::isfinite(pen.y))
			{
				WriteLog("UI", ELogLevel::Error, "UI text line position overflowed.");
				return false;
			}
			continue;
		}

		FFontGlyph glyph;
		if (!element.font->TryGetGlyph(codePoint, glyph)) { return false; }
		if (!ValidateGlyph(glyph)) { return false; }
		if (codePoint != ' ' && glyph.size.x > 0.0f && glyph.size.y > 0.0f)
		{
			const FVector2 position{element.position.x + pen.x + glyph.offset.x * element.scale,
				element.position.y + pen.y + glyph.offset.y * element.scale};
			const FVector2 size{glyph.size.x * element.scale, glyph.size.y * element.scale};
			if (!AppendQuad(glyph.atlas, position, size, glyph.uvMin, glyph.uvMax, element.color, EUISampling::LinearClamp, data)) { return false; }
		}
		pen.x += glyph.advanceX * element.scale;
		if (!std::isfinite(pen.x))
		{
			WriteLog("UI", ELogLevel::Error, "UI text horizontal pen position overflowed.");
			return false;
		}
	}

	return true;
}
