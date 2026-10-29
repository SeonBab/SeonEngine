#pragma once

#include "Core/CoreTypes.h"
#include "Renderer/TextureRenderData.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

class FD3D11Renderer;
class FD3D11ImGuiRendererBackend;

/** D3D11 텍스처의 읽기용 뷰와 선택적인 출력용 뷰를 함께 소유한다. 두 뷰는 같은 색상 자원을 참조한다. */
class FD3D11TextureRenderData : public FTextureRenderData
{
	// 렌더러의 자원 생성/교체와 ImGui backend의 이미지 참조 변환에 내부 접근을 허용한다.
	friend class FD3D11Renderer;
	friend class FD3D11ImGuiRendererBackend;

public:
	/** 생성할 자원의 실제 픽셀 크기를 전달한다. 이 생성자만으로 GPU 자원을 만들지는 않는다. */
	FD3D11TextureRenderData(int32 width, int32 height)
		: FTextureRenderData(width, height)
	{
	}

	/**
	 * 읽기 연결용 SRV를 비소유 포인터로 반환한다. 미준비 상태는 nullptr이며 조회 자체는 AddRef하지 않는다.
	 * 같은 스레드에서 렌더 데이터의 수명 안에서만 빌린다. 교체/비움/파괴 뒤에는 재사용하지 않으며 Release하지 않는다.
	 * 장치 일치, 활성 출력과의 비충돌과 자원의 사용 가능 여부는 호출자가 확인한다. 내부 ComPtr 소유권은 이전하지 않는다.
	 */
	[[nodiscard]] ID3D11ShaderResourceView* GetShaderResourceView() const { return textureView.Get(); }

private:
	// 뷰가 참조하는 색상 자원의 수명도 유지한다. nullptr는 읽기 자원 미준비다.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

	// 일반 이미지에서는 비어 있고 렌더 타겟에서만 준비한다.
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
};
