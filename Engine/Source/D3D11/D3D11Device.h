#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Renderer/Vertex.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

/** D3D11 장치와 즉시 컨텍스트를 소유한다. COM 참조는 명시적 정리 또는 소멸 시 해제한다. */
class FD3D11Device
{
public:
	FD3D11Device() = default;

	FD3D11Device(const FD3D11Device&) = delete;
	FD3D11Device& operator=(const FD3D11Device&) = delete;

	/** 하드웨어 장치를 준비한다. 이미 준비됐거나 생성에 실패하면 false다. */
	bool Initialize();

	/** 즉시 컨텍스트, 장치 순서로 참조를 해제한다. 빈 상태에서도 호출할 수 있다. */
	void Shutdown();

	/** 비소유 장치 포인터다. 참조를 추가하지 않으며 Shutdown/소멸 후에는 사용할 수 없다. */
	ID3D11Device* GetDevice() const { return device.Get(); }

	/** 비소유 즉시 컨텍스트다. Initialize 성공 후 Shutdown/소멸 전까지 사용할 수 있다. */
	ID3D11DeviceContext* GetContext() const { return context.Get(); }

	/** 준비된 장치의 즉시 컨텍스트로 같은 장치의 유효한 RTV를 지운다. RGBA float 4개가 필요하다. */
	void ClearRenderTargetView(ID3D11RenderTargetView* renderTargetView, const float color[4]);

	/** 바이트코드로 정점 셰이더를 만든다. 장치 미준비, 빈 입력, API 실패는 로그/false이며 기존 출력은 유지한다. */
	[[nodiscard]] bool CreateVertexShader(const TArray<uint8>& bytecode, Microsoft::WRL::ComPtr<ID3D11VertexShader>& outShader);

	/** 바이트코드로 픽셀 셰이더를 만든다. 장치 미준비, 빈 입력, API 실패는 로그/false이며 기존 출력은 유지한다. */
	[[nodiscard]] bool CreatePixelShader(const TArray<uint8>& bytecode, Microsoft::WRL::ComPtr<ID3D11PixelShader>& outShader);

	/** 준비된 컨텍스트에 같은 장치의 정점 셰이더를 연결한다. nullptr면 연결을 해제한다. */
	void SetVertexShader(ID3D11VertexShader* shader);

	/** 준비된 컨텍스트에 같은 장치의 픽셀 셰이더를 연결한다. nullptr면 연결을 해제한다. */
	void SetPixelShader(ID3D11PixelShader* shader);

	/** 준비된 컨텍스트에 같은 장치의 RTV 하나를 연결한다. 깊이 버퍼는 사용하지 않으며 nullptr면 연결을 해제한다. */
	void SetRenderTarget(ID3D11RenderTargetView* renderTargetView);

	/** 준비된 컨텍스트에 원점 (0, 0), 깊이 0~1의 뷰포트 하나를 설정한다. 크기는 양수이며 장치 지원 범위여야 한다. */
	void SetViewport(int width, int height);

	/** 준비된 컨텍스트에서 정점을 기본 도형으로 조립하는 방식을 설정한다. */
	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

	/** 준비된 컨텍스트에 비인덱스 그리기를 요청한다. 셰이더, 출력 대상, 뷰포트 등 필요한 상태는 호출자가 설정한다. */
	void Draw(uint32 vertexCount, uint32 startVertexLocation);

	/** 준비된 컨텍스트에 인덱스 그리기를 요청한다. 입력, 셰이더, 출력 상태와 인덱스/정점 범위는 호출자가 보장한다. */
	void DrawIndexed(uint32 indexCount, uint32 startIndexLocation, int32 baseVertexLocation);

	/** 공통 FVertex 정점 배열로 변경 불가 버퍼를 만든다. 장치, 입력, 크기 검사나 API가 실패하면 로그/false이며 기존 출력은 유지한다. */
	[[nodiscard]] bool CreateVertexBuffer(const TArray<FVertex>& vertices, Microsoft::WRL::ComPtr<ID3D11Buffer>& outBuffer);

	/** 32비트 인덱스 배열로 변경 불가 버퍼를 만든다. 장치, 입력, 크기 검사나 API가 실패하면 로그/false이며 기존 출력은 유지한다. */
	[[nodiscard]] bool CreateIndexBuffer(const TArray<uint32>& indices, Microsoft::WRL::ComPtr<ID3D11Buffer>& outBuffer);

	/** FVertex용 입력 레이아웃을 만든다. 장치 초기화가 필요하며 실패 시 기존 출력은 유지한다. */
	[[nodiscard]] bool CreateInputLayout(const TArray<uint8>& vertexShaderBytecode, Microsoft::WRL::ComPtr<ID3D11InputLayout>& outLayout);

	/** 준비된 컨텍스트에 같은 장치의 입력 레이아웃을 연결한다. nullptr면 해제한다. */
	void SetInputLayout(ID3D11InputLayout* layout);

	/** 준비된 컨텍스트의 슬롯 0에 FVertex 정점 버퍼를 연결한다. 시작 오프셋은 0이며 nullptr면 슬롯 연결을 해제한다. */
	void SetVertexBuffer(ID3D11Buffer* buffer);

	/** 슬롯 0에 지정한 정점 간 바이트 간격으로 연결한다. 버퍼와 stride의 일치는 호출자가 보장한다. */
	void SetVertexBuffer(ID3D11Buffer* buffer, uint32 stride);

	/** 준비된 컨텍스트에 같은 장치의 32비트 인덱스 버퍼를 오프셋 0으로 연결한다. nullptr면 해제한다. */
	void SetIndexBuffer(ID3D11Buffer* buffer);

	/** 0보다 크고 최대 64KiB인 16바이트 배수 상수 버퍼를 만든다. 실패하면 로그/false이며 기존 출력은 유지한다. 초기 내용은 미정이다. */
	[[nodiscard]] bool CreateConstantBuffer(uint32 byteSize, Microsoft::WRL::ComPtr<ID3D11Buffer>& outBuffer);

	/** 같은 장치의 DEFAULT 상수 버퍼 전체 갱신을 요청한다. data는 byteSize만큼 읽을 수 있어야 하며 검사 실패 시 내용은 유지한다. true는 GPU 완료를 뜻하지 않는다. */
	[[nodiscard]] bool UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data, uint32 byteSize);

	/** 준비된 컨텍스트의 지정된 VS 슬롯에 같은 장치의 상수 버퍼를 연결한다. nullptr면 연결을 해제한다. */
	void SetVertexShaderConstantBuffer(uint32 slot, ID3D11Buffer* buffer);

	/** 준비된 컨텍스트의 지정된 PS 상수 버퍼 슬롯 1에 같은 장치의 버퍼를 연결한다. nullptr면 연결을 해제한다. */
	void SetPixelShaderConstantBuffer(uint32 slot, ID3D11Buffer* buffer);

	/** 행 패딩 없는 RGBA8 배열로 단일 밉의 변경 불가 텍스처를 만든다. 장치, 크기, 데이터 검사나 API 실패 시 로그/false이며 기존 출력은 유지한다. */
	[[nodiscard]] bool CreateTexture2D(uint32 width, uint32 height, const TArray<uint8>& rgbaPixels, Microsoft::WRL::ComPtr<ID3D11Texture2D>& outTexture);

	/** 같은 장치 텍스처의 기본 SRV를 만든다. 장치 미준비, null, 소속 불일치, API 실패 시 로그/false이며 기존 출력은 유지한다. */
	[[nodiscard]] bool CreateShaderResourceView(ID3D11Texture2D* texture, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& outView);

	/** 호출자가 지정한 설정으로 샘플러를 만든다. 장치 미준비, API 실패 시 로그/false이며 기존 출력은 유지한다. */
	[[nodiscard]] bool CreateSamplerState(const D3D11_SAMPLER_DESC& description, Microsoft::WRL::ComPtr<ID3D11SamplerState>& outState);

	/** 준비된 컨텍스트의 PS t0에 같은 장치의 SRV를 연결한다. 출력 자원과 겹치지 않아야 하며 nullptr면 해제한다. */
	void SetPixelShaderResourceView(ID3D11ShaderResourceView* view);

	/** 준비된 컨텍스트의 PS s0에 같은 장치의 샘플러를 연결한다. nullptr면 명시적 상태를 해제해 D3D11 기본 샘플러를 사용한다. */
	void SetPixelShaderSamplerState(ID3D11SamplerState* sampler);

	/** 호출자가 지정한 설정으로 블렌드 상태를 만든다. 장치 미준비, API 실패 시 로그/false이며 기존 출력은 유지한다. */
	[[nodiscard]] bool CreateBlendState(const D3D11_BLEND_DESC& description, Microsoft::WRL::ComPtr<ID3D11BlendState>& outState);

	/** 준비된 컨텍스트에 같은 장치의 블렌드 상태를 연결한다. nullptr면 불투명 기본 상태로 복원한다. */
	void SetBlendState(ID3D11BlendState* state);

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
};
