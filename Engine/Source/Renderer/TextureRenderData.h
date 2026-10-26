#pragma once

#include "Core/CoreTypes.h"

////////////////////////////////////////////////////////////////////////////////
// 렌더러별 텍스처 GPU 데이터의 공통 소유 경계다. 실제 자원은 파생 타입이 소유한다.
// 크기는 자원 생성 시 정하고 데이터 객체와 함께 교체한다. 객체 자체는 복사하거나 이동하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FTextureRenderData
{
public:
	/** 기반 포인터로 파괴해도 실제 렌더러 데이터의 자원을 해제한다. */
	virtual ~FTextureRenderData();

	FTextureRenderData(const FTextureRenderData&) = delete;
	FTextureRenderData& operator=(const FTextureRenderData&) = delete;
	FTextureRenderData(FTextureRenderData&&) = delete;
	FTextureRenderData& operator=(FTextureRenderData&&) = delete;

	/** 데이터가 나타내는 자원의 실제 너비를 픽셀 단위로 반환한다. GPU 준비 여부는 검사하지 않는다. */
	int32 GetWidth() const { return width; }

	/** 데이터가 나타내는 자원의 실제 높이를 픽셀 단위로 반환한다. GPU 준비 여부는 검사하지 않는다. */
	int32 GetHeight() const { return height; }

protected:
	/** 자원과 일치하는 픽셀 크기를 전달한다. 크기와 GPU 자원의 유효성 검사는 생성하는 렌더러가 맡는다. */
	FTextureRenderData(int32 inWidth, int32 inHeight);

private:
	int32 width = 0;
	int32 height = 0;
};
