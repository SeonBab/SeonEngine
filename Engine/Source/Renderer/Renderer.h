#pragma once

#include "Core/CoreTypes.h"
#include "Core/Math/Matrix4.h"
#include "D3D11/D3D11Device.h"
#include "D3D11/D3D11SwapChain.h"
#include "Renderer/StaticMeshRenderData.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

struct FStaticMeshData;
struct FImage;
struct FUIImageDrawElement;
class FUIDrawElementList;
class UTexture2D;

////////////////////////////////////////////////////////////////////////////////
// 단일 Windows 창의 공통 D3D11 자원과 프레임 그리기를 관리한다. 그릴 GPU 메시는 호출자가 제공한다.
// 창의 생성·메시지 처리는 호출자가 맡고 종료 전 Shutdown을 명시적으로 호출한다.
////////////////////////////////////////////////////////////////////////////////
class FRenderer
{
public:
	FRenderer() = default;

	FRenderer(const FRenderer&) = delete;
	FRenderer& operator=(const FRenderer&) = delete;
	FRenderer(FRenderer&&) = delete;
	FRenderer& operator=(FRenderer&&) = delete;

	/** 유효한 창과 양수 클라이언트 크기로 준비한다. 중복 초기화는 거부하며 자원 생성 실패 시 부분 자원을 정리하고 false를 반환한다. */
	[[nodiscard]] bool Initialize(HWND windowHandle, int width, int height);

	/**
	 * 초기화 후 같은 스레드에서 CPU 데이터를 GPU로 복사한다.
	 * 비어 있지 않은 정점/인덱스와 정점 범위 안의 인덱스를 요구하며 인덱스 개수는 3의 배수여야 한다.
	 * 두 버퍼가 모두 준비된 뒤 출력의 소유권을 교체한다. 실패하면 기존 출력은 유지한다.
	 * 호출자가 자원을 소유하며 CPU 입력은 호출 후 보관할 필요가 없다. 장치가 바뀌면 다시 생성해야 한다.
	 * 이미지는 호출 동안만 빌려 함께 업로드한다. nullptr면 흰색 기본 텍스처를 사용하며 지정한 이미지 생성 실패는 false다.
	 */
	[[nodiscard]] bool CreateMesh(const FStaticMeshData& meshData, FStaticMeshRenderData& outMesh, const FImage* baseColorImage = nullptr);

	/**
	 * 초기화 후 같은 스레드에서 양수 크기의 RGBA8 이미지를 단일 밉의 변경 불가 GPU 텍스처로 복사한다.
	 * 텍스처와 SRV 모두 준비된 뒤 출력의 소유권을 교체한다. 실패하면 기존 출력은 유지한다.
	 * 호출자가 자원을 소유하며 CPU 입력은 성공 후 해제할 수 있다. 장치가 바뀌면 다시 생성해야 한다.
	 */
	[[nodiscard]] bool CreateTexture2D(const FImage& image, UTexture2D& outTexture);

	/**
	 * 초기화 후 같은 스레드에서 객체의 로컬→월드 행렬을 값으로 설정한다. 다음 DrawMesh부터 적용한다.
	 * 보관한 WorldToClip과 합성해 GPU 갱신을 요청하며 실패하면 기존 보관 값을 유지한다.
	 * true는 갱신 요청 성공이며 GPU 완료를 뜻하지 않는다. 행렬 성분의 유효성은 검사하지 않는다.
	 */
	[[nodiscard]] bool SetLocalToWorld(const FMatrix4& matrix);

	/**
	 * 초기화 후 같은 스레드에서 월드→클립 행렬을 값으로 설정한다. 다음 DrawMesh부터 적용한다.
	 * 보관한 LocalToWorld와 합성해 GPU 갱신을 요청하며 실패하면 기존 보관 값을 유지한다.
	 * true는 갱신 요청 성공이며 GPU 완료를 뜻하지 않는다. 행렬 성분의 유효성은 검사하지 않는다.
	 */
	[[nodiscard]] bool SetWorldToClip(const FMatrix4& matrix);

	/** 초기화 후 양수 크기로 프레임을 시작한다. 크기 변화 처리와 화면 지우기를 수행하며 이미 시작한 프레임은 거부한다. */
	[[nodiscard]] bool BeginFrame(int width, int height);

	/** 활성 프레임에서 현재 장치의 GPU 메시를 그린다. 행렬은 현재 설정을 사용하고 기존 정점은 체크 텍스처, 모델은 메시 텍스처/기본 흰색을 사용한다. */
	[[nodiscard]] bool DrawMesh(const FStaticMeshRenderData& mesh);

	/** 목록의 이미지 요소를 순서대로 그린다. 빈 목록은 true이며 첫 실패에서 중단한다. 목록과 텍스처는 보관하지 않는다. */
	[[nodiscard]] bool DrawUI(const FUIDrawElementList& elements);

	/** 시작한 프레임을 닫고 Present한다. 그리기 없는 프레임도 허용하며 Present 실패 시 false를 반환한다. */
	[[nodiscard]] bool EndFrame();

	/** 바인딩·스왑 체인·텍스처/샘플러/버퍼/셰이더·장치 순서로 정리한다. 빈 상태나 부분 초기화 후에도 반복 호출할 수 있다. */
	void Shutdown();

private:
	/**
	 * 활성 프레임에서 현재 장치의 텍스처를 클라이언트 영역 왼쪽 위 기준 픽셀 위치·크기로 그린다.
	 * 위치·크기는 유한하고 크기는 양수여야 하며 행렬 성분의 유한 범위를 넘는 변환은 false다. 음수 위치는 허용하고 화면 밖은 GPU가 자른다.
	 * 월드 변환 데이터는 유지하며 셰이더/버퍼 바인딩은 변경한다. 현재 알파 합성은 지원하지 않는다.
	 * true는 그리기 요청 성공이며 GPU 작업 완료나 화면 표시를 뜻하지 않는다.
	 * 요소의 텍스처 포인터가 비어 있으면 false다. 요소와 텍스처는 호출 동안만 빌린다.
	 */
	[[nodiscard]] bool DrawImage(const FUIImageDrawElement& element);

	FD3D11Device graphics;
	FD3D11SwapChain swapChain;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> transformConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> imageTransformConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> whiteTextureView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> baseSampler;
	// TODO(seon): UI 배칭·전용 정점 도입 시 공통 사각형의 보관 형식을 재검토한다.
	FStaticMeshRenderData imageQuad;
	FMatrix4 localToWorld{};
	FMatrix4 worldToClip{};
	int bufferWidth = 0;
	int bufferHeight = 0;
	bool bInitialized = false;
	bool bFrameActive = false;
};
