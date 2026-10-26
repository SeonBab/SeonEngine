#include "TextureRenderData.h"

FTextureRenderData::~FTextureRenderData() = default;

FTextureRenderData::FTextureRenderData(int32 inWidth, int32 inHeight)
{
	width = inWidth;
	height = inHeight;
}
