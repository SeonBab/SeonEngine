#pragma once

#include "Renderer/D3D11/D3D11Device.h"
#include "Renderer/D3D11/D3D11Viewport.h"
#include "Renderer/RendererInterface.h"

////////////////////////////////////////////////////////////////////////////////
// SeonEngine의 기본 렌더러다. 현재 D3D11 구현으로 단일 창에 출력한다.
// 공통 계약을 구현하며 장치 준비 / 명령은 Device, 창 출력은 Viewport에 맡긴다.
////////////////////////////////////////////////////////////////////////////////
class FRenderer final : public IRenderer
{
public:
	FRenderer() = default;
	~FRenderer() override = default;

	FRenderer(const FRenderer&) = delete;
	FRenderer& operator=(const FRenderer&) = delete;
	FRenderer(FRenderer&&) = delete;
	FRenderer& operator=(FRenderer&&) = delete;

	/** 장치 / 창 출력 / vertex·pixel 셰이더 / 입력 레이아웃 / 변환 상수 버퍼를 준비한다. 출력 / 셰이더 / 레이아웃 / 버퍼 준비 실패 시 부분 자원을 정리하고 false를 반환한다. */
	[[nodiscard]] bool Init(void* WindowHandle, uint32 SizeX, uint32 SizeY) override;

	/** OS 창이 살아 있는 동안 바인딩 / 상수 버퍼·셰이더 자원 / 창 출력 / 장치 순서로 정리한다. 기본 소멸자는 명시적 종료를 대신하지 않는다. */
	void Shutdown() override;

	/** 초기화 성공 후 최소화가 아닌 상태의 양수 클라이언트 크기를 전달한다. */
	void Resize(uint32 SizeX, uint32 SizeY) override;

	/** 목록은 호출 중에만 빌려 쓴다. 메시별 임시 정점 버퍼를 생성하고 월드 행렬을 상수 버퍼에 갱신·VS 슬롯 0에 연결한 뒤 그린다. 생성 / 갱신 실패는 해당 메시를 건너뛴다. 셰이더의 행렬 적용은 아직 없다. */
	void RenderFrame(const TArray<FMeshRenderData>& renderData) override;

private:
	// 현재는 D3D11 고정 구성이다. RHI 도입 시 API 독립 자원 / 명령 계약으로 교체한다.
	FD3D11Device Device;
	FD3D11Viewport Viewport{Device};

	// 현재 단일 셰이더 조합을 보관한다. Init에서 생성하고 매 프레임 연결한다.
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// 메시의 월드 변환 행렬을 전달할 상수 버퍼다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> transformConstantBuffer;
};
