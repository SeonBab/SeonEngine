#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Renderer/FontRenderData.h"

namespace DirectX
{
	inline namespace DX11
	{
		class SpriteFont;
	}
}

class FD3D11Renderer;

/** D3D11에서 사용하는 DirectXTK 폰트 자원을 단독 소유한다. */
class FD3D11FontRenderData : public FFontRenderData
{
	// native 폰트의 외부 접근은 Renderer로 제한한다. 메트릭 조회는 공통 계약으로 제공한다.
	friend class FD3D11Renderer;

public:
	/** 빈 데이터로 생성한다. 파일 읽기와 GPU 자원 생성은 수행하지 않는다. */
	FD3D11FontRenderData();
	~FD3D11FontRenderData() override;

	FD3D11FontRenderData(const FD3D11FontRenderData&) = delete;
	FD3D11FontRenderData& operator=(const FD3D11FontRenderData&) = delete;
	FD3D11FontRenderData(FD3D11FontRenderData&&) = delete;
	FD3D11FontRenderData& operator=(FD3D11FontRenderData&&) = delete;

	/** SpriteFont의 배율 1 줄 간격을 픽셀 단위로 조회한다. 미준비/유효하지 않은 값은 로그/false와 출력 유지다. */
	[[nodiscard]] bool GetLineSpacing(float& outLineSpacing) const override;

	/** BMP 밖의 유효한 코드 포인트도 잘라내지 않고 대체 문자로 조회한다. 실패하면 출력 인자를 유지한다. */
	[[nodiscard]] bool TryGetGlyph(uint32 codePoint, FFontGlyph& outGlyph) const override;

private:
	// nullptr는 폰트 자원 미준비다. 준비된 경우 글리프 정보와 텍스처의 수명을 유지한다.
	TUniquePtr<DirectX::SpriteFont> spriteFont;
};
