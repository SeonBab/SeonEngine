#pragma once

#include "CoreTypes.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

class FRenderer;

////////////////////////////////////////////////////////////////////////////////
// 정적 메시 하나의 GPU 정점·인덱스 버퍼와 선택적인 색상 텍스처 참조를 소유한다.
// 생성·그리기는 렌더러가 수행하며 복사 없이 이동으로 소유권을 전달한다.
////////////////////////////////////////////////////////////////////////////////
class FStaticMeshRenderData
{
	// GPU 자원을 외부에 노출하지 않고 렌더러의 생성·장치 검사·바인딩에만 사용한다.
	friend class FRenderer;

public:
	FStaticMeshRenderData() = default;

	FStaticMeshRenderData(const FStaticMeshRenderData&) = delete;
	FStaticMeshRenderData& operator=(const FStaticMeshRenderData&) = delete;
	FStaticMeshRenderData(FStaticMeshRenderData&&) = default;
	FStaticMeshRenderData& operator=(FStaticMeshRenderData&&) = default;

private:
	enum class EVertexFormat : uint8
	{
		PositionColorUV,
		PositionNormalUV
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	// SRV가 텍스처 참조도 유지하므로 생성 시의 임시 Texture2D 포인터를 따로 보관하지 않는다.
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> baseColorTextureView;
	uint32 indexCount = 0;
	EVertexFormat vertexFormat = EVertexFormat::PositionColorUV;
};
