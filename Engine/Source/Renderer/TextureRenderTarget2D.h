#pragma once

#include "Core/CoreTypes.h"
#include "Renderer/Texture.h"
#include "Renderer/TextureRenderData.h"

/** 색상 결과를 기록하고 셰이더에서 읽는 2D 렌더 타겟이다. 기본 생성만으로 GPU 자원을 만들지는 않는다. */
class UTextureRenderTarget2D : public UTexture
{
public:
	/** 렌더 데이터의 실제 너비를 픽셀 단위로 반환한다. 미지정이면 0이며 출력 가능 여부는 검사하지 않는다. */
	int32 GetWidth() const { return GetRenderData() ? GetRenderData()->GetWidth() : 0; }

	/** 렌더 데이터의 실제 높이를 픽셀 단위로 반환한다. 미지정이면 0이며 출력 가능 여부는 검사하지 않는다. */
	int32 GetHeight() const { return GetRenderData() ? GetRenderData()->GetHeight() : 0; }
};
