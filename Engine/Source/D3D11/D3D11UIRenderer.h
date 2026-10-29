#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

class FD3D11Device;
struct FUIBatchData;
struct FUIRenderBatch;
enum class ETextureAlphaRepresentation : uint8;
enum class EUISampling : uint8;

/**
 * D3D11 UI의 고정/동적 GPU 자원과 그리기 상태 준비를 맡는다.
 * 고정 자원 생성, 동적 정점/인덱스 업로드와 자원 정리, 공통 UI 상태와 배치별 알파/블렌드/샘플러 설정 및 그리기 제출을 제공한다. FD3D11Renderer가 값 멤버로 소유하고 초기화/정리와 UI 제출에서 호출한다.
 */
class FD3D11UIRenderer
{
public:
	FD3D11UIRenderer() = default;

	// ComPtr 멤버의 참조를 해제한다. Shutdown을 자동 호출하지 않는다.
	~FD3D11UIRenderer() = default;

	FD3D11UIRenderer(const FD3D11UIRenderer&) = delete;
	FD3D11UIRenderer& operator=(const FD3D11UIRenderer&) = delete;
	FD3D11UIRenderer(FD3D11UIRenderer&&) = delete;
	FD3D11UIRenderer& operator=(FD3D11UIRenderer&&) = delete;

	/**
	 * 준비된 장치와 같은 스레드에서 고정 UI 자원을 생성한다. 중복 초기화, 미준비 장치/컨텍스트와 빈 입력을 거부한다.
	 * 입력 배열은 호출 동안만 읽으며 복사하거나 보관하지 않는다. 셰이더 2개와 FUIVertex 입력 레이아웃을 생성한다.
	 * 변환 상수 버퍼(64바이트)를 생성하고 단위 행렬을 기록한다. 실제 픽셀->클립 행렬 계산과 바인딩은 하지 않는다.
	 * 텍스처 알파 표현 상수 버퍼(16바이트)는 Straight와 패딩 0으로 초기화한다. PS b0에 연결하지 않는다.
	 * Straight/Premultiplied 알파 블렌드 상태를 각각 생성한다. GPU에 연결하거나 실제 합성을 수행하지 않는다.
	 * 깊이 검사/쓰기와 스텐실 검사를 끈 UI 깊이 상태를 생성한다. GPU에 연결하지 않는다.
	 * UI 전용 PointClamp/LinearClamp 샘플러를 생성한다. 메시 샘플러를 빌리거나 PS s0에 연결하지 않는다.
	 * 모두 준비되면 자원을 유지하고 true를 반환한다. 입력/중복 검사 실패는 기존 상태를 유지한다.
	 * 생성/초기값 기록 실패는 Shutdown으로 부분 자원을 정리하고 false를 반환한다. Shutdown 후 재초기화할 수 있다.
	 * true는 고정 자원 준비 완료이며 GPU 바인딩, 실제 UI 그리기나 GPU 완료를 뜻하지 않는다.
	 */
	[[nodiscard]] bool Initialize(FD3D11Device& graphics, const TArray<uint8>& vertexShaderBytecode, const TArray<uint8>& pixelShaderBytecode);

	/**
	 * 소유한 UI 고정/동적 자원의 COM 참조를 해제하고 버퍼 용량과 초기화 상태를 지운다. 빈 상태, 부분 초기화와 반복 호출을 허용한다.
	 * 장치 종료 전에 호출한다. GPU 바인딩 해제는 상위 렌더러가 맡으며 이 함수는 컨텍스트 상태를 변경하지 않는다.
	 */
	void Shutdown();

	/**
	 * 초기화 후 CPU 배치의 정점과 인덱스를 UI 버퍼에 업로드한다. 두 배열이 모두 비면 버퍼를 유지하고 성공한다.
	 * 초기화 때와 같은 장치를 같은 스레드에서 사용한다. CPU 배열은 호출 동안만 읽으며 보관하지 않는다.
	 * 한쪽만 비거나 byte 크기가 uint32 범위를 넘으면 로그/false다. 인덱스 값, 배치 범위와 텍스처의 유효성은 호출자가 보장한다.
	 * 용량 확보 뒤 정점, 인덱스 순서로 WRITE_DISCARD 갱신한다. 중간 실패 시 일부 버퍼가 이미 교체되거나 갱신됐을 수 있다.
	 * false면 이번 배치를 제출하지 않아야 한다. 바인딩과 그리기는 하지 않으며 true는 GPU 완료를 뜻하지 않는다.
	 */
	[[nodiscard]] bool UploadBatchData(FD3D11Device& graphics, const FUIBatchData& batchData);

	/**
	 * 초기화된 객체와 양수 출력 픽셀 크기로 변환을 갱신한 뒤 UI 깊이 상태/셰이더/입력 레이아웃/VS b0/PS b0/정점·인덱스 버퍼/TRIANGLELIST를 연결한다.
	 * 초기화 때와 같은 장치를 같은 스레드에서 사용한다. 입력/갱신 실패는 바인딩 전에 false이며 자원과 기존 연결을 유지한다.
	 * 비어 있지 않은 유효한 배치의 UploadBatchData가 먼저 성공해야 한다. 버퍼 미준비 시 연결/상수 갱신 전에 false다. 업로드 실패 뒤 호출하면 안 된다.
	 * true는 공통 상태와 geometry 연결 완료이며 GPU 완료나 그리기를 뜻하지 않는다. 텍스처와 배치별 상태는 호출자가 준비한다. 출력 선택, clear와 프레임 시작은 하지 않는다. 연결된 상태를 자동 복원하지 않으며 해제/복원은 상위 렌더러 책임이다.
	 */
	[[nodiscard]] bool PrepareDraw(FD3D11Device& graphics, int32 outputWidth, int32 outputHeight);

	/**
	 * 초기화 여부와 지원하는 알파 표현/샘플링을 검사하고 PS 알파 상수 버퍼를 갱신한 뒤 블렌드와 PS s0 샘플러를 연결한다.
	 * 초기화 때와 같은 장치를 같은 스레드에서 사용한다. 입력/갱신 실패는 기존 상수와 연결을 유지하며 자원을 정리하지 않는다.
	 * 알파 플래그와 패딩을 포함한 16바이트 전체를 기록한다. 패딩은 항상 0이다.
	 * true는 배치 상태 설정 완료이며 GPU 완료를 뜻하지 않는다. 텍스처 연결과 그리기는 하지 않는다.
	 * 연결된 상태를 자동 복원하지 않으며 해제/복원은 상위 렌더러 책임이다.
	 */
	[[nodiscard]] bool SetBatchState(FD3D11Device& graphics, ETextureAlphaRepresentation alphaRepresentation, EUISampling sampling);

	/**
	 * 전체 배치 검증과 UploadBatchData/PrepareDraw 성공 뒤 배치 하나의 상태/텍스처를 연결하고 DrawIndexed를 제출한다.
	 * 초기화 때와 같은 장치를 같은 스레드에서 사용한다. 유효한 D3D11 텍스처/SRV, 같은 장치, 출력과의 비충돌, 인덱스 범위는 호출자가 보장한다.
	 * 배치와 텍스처/렌더 데이터는 호출 동안 빌리며 보관하지 않는다. 전체 제출 완료까지 자원과 메타데이터를 유지하고 교체하지 않는다.
	 * SetBatchState 실패는 텍스처 연결/Draw 전에 false다. 이전 배치의 픽셀은 롤백하지 않으며 상태 복원은 상위 렌더러 책임이다.
	 * true는 제출 완료이며 GPU 완료를 뜻하지 않는다.
	 */
	[[nodiscard]] bool SubmitBatch(FD3D11Device& graphics, const FUIRenderBatch& batch);

private:
	/**
	 * 초기화 후 UI 정점/인덱스 버퍼를 필요한 byte 용량 이상으로 확보한다. 0인 요청은 해당 버퍼를 유지한다.
	 * 배열 개수를 byte 크기로 환산할 때의 범위 검사는 이 함수를 호출하기 전에 수행해야 한다.
	 * 기존 용량이 충분하면 재사용하고 부족하면 두 배와 요청 크기 중 큰 값으로 확장하며 uint32 최대값을 넘지 않는다.
	 * 필요한 새 버퍼가 모두 준비된 후 교체한다. 실패하면 기존 두 버퍼와 용량을 유지한다.
	 * 새 버퍼로 이전 내용을 복사하지 않는다. 데이터 업로드, 바인딩과 그리기는 수행하지 않는다.
	 */
	[[nodiscard]] bool EnsureUIBufferCapacity(FD3D11Device& graphics, uint32 requiredVertexBytes, uint32 requiredIndexBytes);

	/** 부족한 용량만 확장하며 byte 단위 결과를 uint32 범위에 제한한다. */
	static uint32 CalculateUIBufferCapacity(uint32 currentCapacity, uint32 requiredBytes);

	// 모든 ComPtr는 기본 생성 시 비어 있다. 멤버 선언만으로 GPU 자원을 생성하지 않는다.
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	// FUIVertex의 위치(float2), UV(float2), Straight 색상(float4)에 대응한다.
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// UI VS b0: 출력 크기에 따른 row_major 픽셀->클립 변환 행렬(64바이트).
	Microsoft::WRL::ComPtr<ID3D11Buffer> transformConstantBuffer;

	// UI PS b0: 텍스처의 Straight/Premultiplied 알파 표현과 패딩(16바이트).
	Microsoft::WRL::ComPtr<ID3D11Buffer> textureConstantBuffer;

	// Straight 입력의 RGB에 알파를 곱해 배경과 합성한다.
	Microsoft::WRL::ComPtr<ID3D11BlendState> straightBlendState;

	// Premultiplied 입력의 RGB에는 알파를 다시 곱하지 않는다.
	Microsoft::WRL::ComPtr<ID3D11BlendState> premultipliedBlendState;

	// UI를 그릴 때 깊이 검사와 깊이 쓰기를 끈다.
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;

	// UI 전용 샘플러다. 메시 샘플러 배열의 소유나 수명에 의존하지 않는다.
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pointClampSamplerState;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> linearClampSamplerState;

	// UI 배치의 정점과 인덱스를 여러 제출에서 재사용하기 위한 소유 버퍼다. 미생성 상태는 nullptr다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> uiVertexBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> uiIndexBuffer;

	// 확보한 GPU 공간의 바이트 수다. 사용 중인 원소 개수와 구분하며 미생성 상태는 0이다.
	uint32 uiVertexBufferByteCapacity = 0;

	uint32 uiIndexBufferByteCapacity = 0;

	// Initialize가 모든 고정 자원을 준비한 뒤에만 true이며 Shutdown에서 false로 바꾼다.
	bool bInitialized = false;
};
