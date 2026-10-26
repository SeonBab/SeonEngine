#pragma once

#include "Core/CoreTypes.h"
#include "Renderer/StaticMeshRenderData.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

class FD3D11Renderer;

////////////////////////////////////////////////////////////////////////////////
// D3D11 정적 메시의 정점, 인덱스 버퍼와 인덱스 개수를 소유한다.
// 기반 포인터로 소유해도 파괴 시 두 버퍼의 COM 참조를 해제한다.
////////////////////////////////////////////////////////////////////////////////
class FD3D11StaticMeshRenderData : public FStaticMeshRenderData
{
	// GPU 자원을 외부에 노출하지 않고 렌더러의 생성, 장치 검사, 바인딩에만 사용한다.
	friend class FD3D11Renderer;

public:
	FD3D11StaticMeshRenderData() = default;

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	uint32 indexCount = 0;
};
