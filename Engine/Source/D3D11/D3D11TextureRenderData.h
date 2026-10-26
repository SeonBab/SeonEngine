#pragma once

#include "Core/CoreTypes.h"
#include "Renderer/TextureRenderData.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

class FD3D11Renderer;

/** D3D11 텍스처의 읽기용 뷰와 선택적인 출력용 뷰를 함께 소유한다. 두 뷰는 같은 색상 자원을 참조한다. */
class FD3D11TextureRenderData : public FTextureRenderData
{
	// 뷰를 외부에 노출하지 않고 렌더러의 생성, 장치 검사, 바인딩에만 사용한다.
	friend class FD3D11Renderer;

public:
	/** 생성할 자원의 실제 픽셀 크기를 전달한다. 이 생성자만으로 GPU 자원을 만들지는 않는다. */
	FD3D11TextureRenderData(int32 width, int32 height)
		: FTextureRenderData(width, height)
	{
	}

private:
	// 뷰가 참조하는 색상 자원의 수명도 유지한다. nullptr는 읽기 자원 미준비다.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	// 일반 이미지에서는 비어 있고 렌더 타겟에서만 준비한다.
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
};
