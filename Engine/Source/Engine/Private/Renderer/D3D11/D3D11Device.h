#pragma once

#include "Containers/Array.h"
#include "CoreTypes.h"
#include "Math/Vector3.h"
#include "Platform/Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

////////////////////////////////////////////////////////////////////////////////
// D3D11 장치와 즉시 컨텍스트를 관리한다. 창 출력 자원은 Viewport가 관리한다.
// 장치와 컨텍스트의 종료 책임이 하나여야 하므로 복사와 이동을 금지한다.
////////////////////////////////////////////////////////////////////////////////
class FD3D11Device
{
public:
	FD3D11Device() = default;
	~FD3D11Device() = default;

	FD3D11Device(const FD3D11Device&) = delete;
	FD3D11Device& operator=(const FD3D11Device&) = delete;
	FD3D11Device(FD3D11Device&&) = delete;
	FD3D11Device& operator=(FD3D11Device&&) = delete;

	/**
	 * D3D11 장치와 즉시 컨텍스트를 준비한다. 장치가 이미 있으면 재사용한다.
	 * 정상 반환 시 장치가 준비돼 있다. 생성 실패는 부분 자원 정리 후 치명 종료한다.
	 */
	void InitD3DDevice();

	/**
	 * 컨텍스트 바인딩과 보유 COM 참조를 정리한다. GPU 완료를 기다리는 함수는 아니다.
	 * 정상 종료 전에 장치를 사용하는 모든 Viewport와 다른 GPU 자원을 정리해야 한다.
	 * 기본 소멸자는 COM 참조만 해제하므로 정상 종료의 명시적 호출을 대신하지 않는다.
	 */
	void Shutdown();

	/**
	 * 준비된 즉시 컨텍스트로 지정한 RTV 전체를 RGBA 색상으로 지운다. 화면 제출은 하지 않는다.
	 * 장치 준비 후 호출하며 유효한 RTV와 float 네 개의 색상 배열이 필요하다.
	 * RTV와 색상은 호출 동안만 빌려 쓰며 소유하거나 해제하지 않는다.
	 */
	void ClearRenderTargetView(ID3D11RenderTargetView* InRenderTargetView, const float InClearColor[4]);

	/**
	 * 지정한 바이트 크기의 상수 버퍼를 생성한다. 크기는 0이 아닌 16바이트 배수여야 한다.
	 * 장치 부재 / 지원 크기 초과 / 생성 실패는 로그와 false로 알리고 기존 출력을 유지한다.
	 * 성공 시에만 출력을 새 소유 참조로 교체한다. 호출자는 장치 종료 전에 참조를 해제해야 한다.
	 * 생성만으로 내용이 초기화되지 않는다. 데이터 갱신 / 셰이더 바인딩 / 그리기는 별도다.
	 */
	[[nodiscard]] bool CreateConstantBuffer(uint32 byteSize, Microsoft::WRL::ComPtr<ID3D11Buffer>& outConstantBuffer);

	/**
	 * 같은 장치의 DEFAULT 상수 버퍼 전체를 CPU 데이터로 갱신한다. 데이터는 호출 동안만 빌린다.
	 * data는 byteSize만큼 읽을 수 있어야 하며 크기는 버퍼 전체 바이트 크기와 같아야 한다.
	 * 컨텍스트 부재 / 빈 버퍼·데이터 / 용도·크기 불일치는 로그와 false로 알리고 갱신하지 않는다.
	 * true는 입력 검사를 통과하고 갱신 호출을 수행했음을 뜻하며 GPU 완료 / 모든 실행 오류 검출은 아니다.
	 * 버퍼 소유권 이전 / 셰이더 바인딩 / 그리기는 하지 않는다.
	 */
	[[nodiscard]] bool UpdateConstantBuffer(ID3D11Buffer* constantBuffer, const void* data, uint32 byteSize);

	/**
	 * 준비된 컨텍스트의 vertex shader 상수 버퍼 슬롯 0에 버퍼 하나를 연결한다.
	 * 같은 장치의 유효한 상수 버퍼가 필요하며 nullptr는 슬롯 0 연결 해제다.
	 * Device 멤버에는 저장하지 않지만 컨텍스트는 바인딩 동안 COM 참조를 보유한다.
	 * 데이터 갱신 / 다른 슬롯·셰이더 단계 설정 / 그리기는 하지 않는다.
	 */
	void SetVertexShaderConstantBuffer(ID3D11Buffer* constantBuffer);

	/**
	 * 로컬 정점 위치를 복사한 변경 불가 GPU 버퍼를 생성한다. CPU 배열은 호출 중에만 필요하다.
	 * 빈 입력은 생성 없이 false다. 장치 부재 / 크기 초과 / 생성 실패도 false이며 원인을 로그로 남긴다.
	 * 성공 시에만 출력을 새 소유 참조로 교체한다. 호출자는 장치 종료 전에 버퍼 참조를 해제해야 한다.
	 * 정점 입력 바인딩 / 그리기 / 월드 행렬 전달은 하지 않는다.
	 */
	[[nodiscard]] bool CreateVertexBuffer(const TArray<FVector3>& vertices, Microsoft::WRL::ComPtr<ID3D11Buffer>& outVertexBuffer);

	/**
	 * 준비된 컨텍스트의 입력 슬롯 0에 FVector3 간격 / 시작 오프셋 0으로 정점 버퍼 하나를 연결한다.
	 * 이 장치에서 만든 정점 버퍼가 필요하며 nullptr는 슬롯 0 연결 해제다. Draw는 하지 않는다.
	 * Device 멤버에는 저장하지 않지만 컨텍스트는 바인딩 동안 COM 참조를 보유한다.
	 */
	void SetVertexBuffer(ID3D11Buffer* vertexBuffer);

	/**
	 * 컴파일된 vertex shader 입력에 맞춰 슬롯 0의 POSITION0 / float3 레이아웃을 생성한다.
	 * 바이트코드는 호출 동안만 필요하다. 빈 입력 / 장치 부재 / API 실패는 로그와 false로 알린다.
	 * 성공 시에만 출력을 새 소유 참조로 교체한다. 호출자는 장치 종료 전에 참조를 해제해야 한다.
	 * 레이아웃 연결 / 셰이더 객체 생성 / 그리기는 하지 않는다.
	 */
	[[nodiscard]] bool CreateInputLayout(const TArray<uint8>& vertexShaderBytecode, Microsoft::WRL::ComPtr<ID3D11InputLayout>& outInputLayout);

	/**
	 * 준비된 컨텍스트의 입력 레이아웃을 선택한다. 같은 장치의 레이아웃이 필요하며 nullptr는 해제다.
	 * Device 멤버에는 저장하지 않지만 컨텍스트가 바인딩 동안 COM 참조를 보유한다. Draw는 하지 않는다.
	 */
	void SetInputLayout(ID3D11InputLayout* inputLayout);

	/**
	 * 컴파일된 바이트코드로 vertex shader 객체를 생성한다. 바이트코드는 호출 동안만 필요하다.
	 * 빈 입력 / 장치 부재 / API 실패는 로그와 false로 알리고 기존 출력을 유지한다.
	 * 성공 시 출력을 새 소유 참조로 교체한다. 호출자는 장치 종료 전에 참조를 해제해야 한다.
	 * 동적 셰이더 연결 / 파일 읽기 / 셰이더 바인딩 / 그리기는 하지 않는다.
	 */
	[[nodiscard]] bool CreateVertexShader(const TArray<uint8>& vertexShaderBytecode, Microsoft::WRL::ComPtr<ID3D11VertexShader>& outVertexShader);

	/**
	 * 준비된 컨텍스트에 같은 장치의 vertex shader를 선택한다. nullptr는 해당 단계의 셰이더 해제다.
	 * 클래스 인스턴스 없는 셰이더만 사용한다. 컨텍스트는 바인딩 동안 COM 참조를 보유한다.
	 * Device 멤버에 저장하거나 Draw를 호출하지 않는다.
	 */
	void SetVertexShader(ID3D11VertexShader* vertexShader);

	/**
	 * 컴파일된 바이트코드로 pixel shader 객체를 생성한다. 바이트코드는 호출 동안만 필요하다.
	 * 빈 입력 / 장치 부재 / API 실패는 로그와 false로 알리고 기존 출력을 유지한다.
	 * 성공 시 출력을 새 소유 참조로 교체한다. 호출자는 장치 종료 전에 참조를 해제해야 한다.
	 * 동적 셰이더 연결 / 파일 읽기 / 셰이더 바인딩 / 그리기는 하지 않는다.
	 */
	[[nodiscard]] bool CreatePixelShader(const TArray<uint8>& pixelShaderBytecode, Microsoft::WRL::ComPtr<ID3D11PixelShader>& outPixelShader);

	/**
	 * 준비된 컨텍스트에 같은 장치의 pixel shader를 선택한다. nullptr는 해당 단계의 셰이더 해제다.
	 * 클래스 인스턴스 없는 셰이더만 사용한다. 컨텍스트는 바인딩 동안 COM 참조를 보유한다.
	 * Device 멤버에 저장하거나 Draw를 호출하지 않는다.
	 */
	void SetPixelShader(ID3D11PixelShader* pixelShader);

	/**
	 * 준비된 컨텍스트에 정점 조립 규칙을 설정한다. 사용할 파이프라인에 맞는 유효한 값이 필요하다.
	 * 버퍼 / 셰이더를 연결하거나 Draw를 호출하지 않는다. 자원을 생성하거나 소유하지 않는다.
	 */
	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

	/**
	 * 비인덱스 / 비인스턴스 그리기 명령을 전달한다. 시작 정점 번호와 사용할 정점 수를 받는다.
	 * 준비된 컨텍스트와 유효한 정점 범위 / 파이프라인 상태가 필요하며 호출자가 설정한다.
	 * 상태 설정 / 화면 제출 / GPU 완료 대기는 하지 않는다.
	 */
	void Draw(uint32 vertexCount, uint32 startVertexLocation);

	/**
	 * 준비된 컨텍스트에 같은 장치의 색상 RTV 하나를 선택한다. nullptr는 출력 연결 해제다.
	 * 다른 색상 슬롯과 깊이 타깃도 해제한다. 컨텍스트는 연결한 RTV의 COM 참조를 유지한다.
	 * 자원 생성 / Clear / 화면 영역 설정 / Draw는 하지 않는다.
	 */
	void SetRenderTargetView(ID3D11RenderTargetView* renderTargetView);

	/**
	 * 준비된 컨텍스트에 화면 영역 하나와 깊이 범위를 설정한다. 다른 viewport는 비활성화한다.
	 * 유효한 값은 호출자가 준비한다. 입력은 호출 중에만 필요하며 보관하거나 수정하지 않는다.
	 * 출력 타깃 / scissor / Draw는 설정하지 않는다.
	 */
	void SetViewport(const D3D11_VIEWPORT& viewport);

	/** 준비된 장치의 비소유 포인터를 반환한다. 생성 전과 정리 후에는 nullptr이다. */
	ID3D11Device* GetDevice() const { return Direct3DDevice.Get(); }

	/**
	 * 보관한 즉시 컨텍스트의 비소유 포인터를 반환한다. 준비 전과 종료 후에는 nullptr이다.
	 * 반환 주소는 장치가 컨텍스트 참조를 해제하기 전까지만 사용한다.
	 */
	ID3D11DeviceContext* GetDeviceContext() const;

private:
	Microsoft::WRL::ComPtr<ID3D11Device> Direct3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Direct3DDeviceIMContext;
};
