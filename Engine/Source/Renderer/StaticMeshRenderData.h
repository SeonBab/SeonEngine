#pragma once

#include "Core/CoreTypes.h"
#include "Renderer/Texture2D.h"
#include "Renderer/TextureAddressMode.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

class FRenderer;

////////////////////////////////////////////////////////////////////////////////
// 정적 메시 하나의 GPU 정점, 인덱스 버퍼와 선택적인 색상 텍스처 참조를 소유한다.
// U와 V의 주소 모드를 함께 보관하며 생성, 그리기는 렌더러가 수행한다. 복사 없이 이동으로 소유권을 전달한다.
////////////////////////////////////////////////////////////////////////////////
class FStaticMeshRenderData
{
	// GPU 자원을 외부에 노출하지 않고 렌더러의 생성, 장치 검사, 바인딩에만 사용한다.
	friend class FRenderer;

public:
	FStaticMeshRenderData() = default;

	FStaticMeshRenderData(const FStaticMeshRenderData&) = delete;
	FStaticMeshRenderData& operator=(const FStaticMeshRenderData&) = delete;
	FStaticMeshRenderData(FStaticMeshRenderData&&) = default;
	FStaticMeshRenderData& operator=(FStaticMeshRenderData&&) = default;

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	UTexture2D baseColorTexture;
	uint32 indexCount = 0;
	ETextureAddressMode addressU = ETextureAddressMode::Clamp;
	ETextureAddressMode addressV = ETextureAddressMode::Clamp;
};
