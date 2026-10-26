#pragma once

#include "Core/CoreTypes.h"
#include "Core/Math/Matrix4.h"
#include "Core/Math/Vector3.h"
#include "Core/Templates/UniquePtr.h"
#include "D3D11/D3D11Device.h"
#include "D3D11/D3D11SwapChain.h"
#include "Renderer/Renderer.h"
#include "Renderer/StaticMeshRenderData.h"
#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

#include <array>

struct FStaticMeshData;
struct FImage;
struct FUIImageDrawElement;
struct FMeshDrawItem;
struct FSceneRenderData;
struct FRenderView;
class FUIDrawElementList;
class UTexture2D;
class UTextureRenderTarget2D;

////////////////////////////////////////////////////////////////////////////////
// 단일 Windows 창의 공통 D3D11 자원과 프레임 그리기를 관리한다. 그릴 GPU 메시는 호출자가 제공한다.
// 창의 생성과 메시지 처리는 호출자가 맡고 종료 전 Shutdown을 명시적으로 호출한다.
////////////////////////////////////////////////////////////////////////////////
class FD3D11Renderer : public IRenderer
{
#pragma region Lifecycle

public:
	FD3D11Renderer() = default;

	~FD3D11Renderer() override = default;

	FD3D11Renderer(const FD3D11Renderer&) = delete;

	FD3D11Renderer& operator=(const FD3D11Renderer&) = delete;

	FD3D11Renderer(FD3D11Renderer&&) = delete;

	FD3D11Renderer& operator=(FD3D11Renderer&&) = delete;

	/** 유효한 창과 양수 클라이언트 크기로 준비한다. 중복 초기화는 거부하며 자원 생성 실패 시 부분 자원을 정리하고 false를 반환한다. */
	[[nodiscard]] bool Initialize(void* nativeWindowHandle, int32 width, int32 height) override;

	/** 초기화된 장치에 맞는 미초기화 backend를 만든다. 반환 객체는 이 Renderer보다 먼저 정리해야 한다. */
	[[nodiscard]] TUniquePtr<IImGuiRendererBackend> CreateImGuiRendererBackend() override;

	/** 바인딩, 스왑 체인, 텍스처/샘플러/버퍼/셰이더, 장치 순서로 정리한다. 빈 상태나 부분 초기화 후에도 반복 호출할 수 있다. */
	void Shutdown() override;

	/** 초기화 성공 후 종료 전 같은 스레드에서 빌려 사용한다. COM 참조를 추가하지 않는다. */
	[[nodiscard]] ID3D11Device* GetD3D11Device() const { return graphics.GetDevice(); }

	/** 초기화 성공 후 종료 전 같은 스레드에서 빌려 사용한다. COM 참조를 추가하지 않는다. */
	[[nodiscard]] ID3D11DeviceContext* GetD3D11DeviceContext() const { return graphics.GetContext(); }

#pragma endregion

#pragma region ResourceCreation

public:
	/**
	 * 초기화 후 같은 스레드에서 CPU 데이터를 GPU로 복사한다.
	 * 비어 있지 않은 정점/인덱스와 정점 범위 안의 인덱스를 요구하며 인덱스 개수는 3의 배수여야 한다.
	 * 두 버퍼가 모두 준비된 뒤 출력의 소유권을 교체한다. 실패하면 기존 출력은 유지한다.
	 * 성공하면 이전 출력 객체가 파괴되므로 기존 비소유 사용을 끝낸 뒤 호출하고 이전 조회 주소는 재사용하지 않는다.
	 * 호출자가 자원을 소유하며 CPU 입력은 호출 후 보관할 필요가 없다. 장치가 바뀌면 다시 생성해야 한다.
	 * 형상만 생성한다. CPU 입력의 재질 번호와 주소 설정은 사용하지 않으며 텍스처는 CreateTexture2D로 별도 준비한다.
	 */
	[[nodiscard]] bool CreateMesh(const FStaticMeshData& meshData, TUniquePtr<FStaticMeshRenderData>& outMesh) override;

	/**
	 * 초기화 후 같은 스레드에서 양수 크기의 RGBA8 이미지를 GPU로 복사하고 전체 하위 밉 생성을 요청한다.
	 * 텍스처와 SRV 모두 준비된 뒤 렌더 데이터와 실제 크기를 교체하며 자산 주소와 AddressX/Y 설정은 유지한다.
	 * 실패하면 기존 렌더 데이터와 주소 설정을 모두 유지한다. 성공 교체 전 기존 데이터의 비소유 사용을 끝내야 한다.
	 * 호출자가 자원을 소유하며 CPU 입력은 성공 후 해제할 수 있다. 장치가 바뀌면 다시 생성해야 한다.
	 */
	[[nodiscard]] bool CreateTexture2D(const FImage& image, UTexture2D& outTexture) override;

	/**
	 * 초기화 후 같은 스레드의 활성 프레임 밖에서 RGBA8 UNORM 렌더 타겟을 만든다.
	 * 가로, 세로는 각각 1~16384이며 단일 밉, 단일 샘플이다. SRV와 RTV를 준비하고 검정색, 알파 1로 초기화를 요청한다.
	 * 모두 준비된 뒤 렌더 데이터와 실제 크기를 함께 교체하고 자산 주소와 AddressX/Y는 유지한다. 실패하면 기존 출력은 유지한다.
	 * 성공 교체 전 기존 데이터의 비소유 사용을 끝내야 한다.
	 * 깊이 자원, 출력 바인딩, 뷰포트는 변경하지 않는다. true는 GPU 완료나 화면 표시를 뜻하지 않는다.
	 * 호출자가 객체와 자원을 소유하며 장치가 바뀌면 다시 생성해야 한다.
	 */
	[[nodiscard]] bool CreateRenderTarget2D(int32 width, int32 height, UTextureRenderTarget2D& outTexture) override;

#pragma endregion

#pragma region ViewTransform

private:
	/**
	 * 초기화 후 같은 스레드에서 객체를 로컬에서 월드로 변환하는 행렬을 값으로 설정한다. 다음 DrawMesh부터 적용한다.
	 * 보관한 WorldToClip과 합성해 GPU 갱신을 요청하며 실패하면 기존 보관 값을 유지한다.
	 * true는 갱신 요청 성공이며 GPU 완료를 뜻하지 않는다. 행렬 성분의 유효성은 검사하지 않는다.
	 */
	[[nodiscard]] bool SetLocalToWorld(const FMatrix4& matrix);

	/**
	 * 초기화 후 같은 스레드에서 월드에서 클립 공간으로 변환하는 행렬을 값으로 설정한다. 다음 DrawMesh부터 적용한다.
	 * 보관한 LocalToWorld와 합성해 GPU 갱신을 요청하며 실패하면 기존 보관 값을 유지한다.
	 * true는 갱신 요청 성공이며 GPU 완료를 뜻하지 않는다. 행렬 성분의 유효성은 검사하지 않는다.
	 */
	[[nodiscard]] bool SetWorldToClip(const FMatrix4& matrix);

#pragma endregion

#pragma region Lighting

private:
	/**
	 * 초기화 후 같은 스레드에서 방향광 버퍼 갱신을 요청한다.
	 * 방향은 빛이 진행하는 월드 단위 방향이며 색은 세기가 반영된 유한한 0 이상 선형 RGB다.
	 * 입력 검사나 정규화는 하지 않는다. RGB가 0이면 방향광 기여가 없다.
	 */
	[[nodiscard]] bool SetDirectionalLight(FVector3 lightDirection, FVector3 lightColor);

	/**
	 * 초기화 후 같은 스레드에서 SkyLight의 균일 주변광 버퍼 갱신을 요청한다.
	 * 색은 세기가 반영된 유한한 0 이상 선형 RGB다. 검사/clamp/세기 재곱셈은 하지 않는다.
	 * RGB 0은 주변광 기여 없음이며 다음 DrawMesh부터 적용한다. UI에는 적용하지 않는다.
	 */
	[[nodiscard]] bool SetSkyLight(FVector3 lightColor);

#pragma endregion

#pragma region Frame

public:
	/** 초기화 후 양수 크기로 프레임을 시작한다. 크기 변화 처리와 화면 지우기를 수행하며 이미 시작한 프레임은 거부한다. */
	[[nodiscard]] bool BeginFrame(int32 width, int32 height) override;

	/**
	 * 활성 프레임에서 같은 D3D11 장치의 SRV와 RTV가 준비된 렌더 타겟으로 출력을 전환한다.
	 * 타겟 크기의 깊이 버퍼와 뷰포트를 연결하고 검정색, 알파 1과 깊이 1로 지운다.
	 * 중첩 호출은 거부하며 준비 실패 시 기존 출력과 변환 행렬을 유지한다.
	 * 성공 후 EndRenderTarget으로 창 출력과 기존 변환 행렬을 복원해야 한다.
	 * true는 출력 전환과 초기화 요청 성공이며 GPU 완료를 뜻하지 않는다.
	 */
	[[nodiscard]] bool BeginRenderTarget(const UTextureRenderTarget2D& target) override;

	/**
	 * 활성 렌더 타겟 출력을 끝내고 창 출력과 저장한 변환 행렬을 복원한다.
	 * 창의 내용은 지우지 않고 Present도 하지 않는다.
	 * 복원 실패 시 false를 반환하며 렌더 타겟 활성 상태를 유지한다.
	 * true는 복원 요청 성공이며 GPU 완료를 뜻하지 않는다.
	 */
	[[nodiscard]] bool EndRenderTarget() override;

	/**
	 * 활성 프레임에서 시점·광원을 적용하고 요청 배열 순서대로 메시를 그린다. 출력 선택·clear·Present는 하지 않는다.
	 * 호출 동안 요청과 자원을 빌리며 보관하지 않는다. 처리 중 자원 교체·파괴나 재진입 변경을 하지 않는다.
	 * 형상 미지정, 다른 구현·장치 또는 활성 출력과 같은 자원 읽기는 거부한다. 텍스처 미준비는 흰색 대체다.
	 * 실패 전 적용·제출한 작업은 롤백하지 않는다. true도 GPU 완료나 화면 표시를 보장하지 않는다.
	 */
	[[nodiscard]] bool DrawScene(const FSceneRenderData& data) override;

	/**
	 * 시작한 프레임을 닫고 Present한다. 그리기 없는 프레임도 허용한다.
	 * 렌더 타겟 활성 중에는 상태를 유지하고 false로 거부한다. EndRenderTarget 후 다시 호출한다.
	 * Present 실패 시에도 false를 반환하며 이 경우 프레임은 이미 종료된 상태다.
	 */
	[[nodiscard]] bool EndFrame() override;

private:
	/** 준비된 프레임에서 형상·텍스처의 구현·장치를 검사하고 재질·시점 값을 VS b3에 업로드한 뒤 그린다. 텍스처 X/Y로 샘플러를 선택하며 미준비는 흰색 대체다. 시점은 스케일 없는 뷰를 요구한다. */
	[[nodiscard]] bool DrawMesh(const FMeshDrawItem& item, const FRenderView& view);

	/** SRV가 활성 렌더 타겟과 같은 원본 자원을 가리키면 true다. 비활성 상태나 nullptr 입력은 false다. */
	[[nodiscard]] bool IsActiveRenderTargetResource(ID3D11ShaderResourceView* view) const;

#pragma endregion

#pragma region UI

public:
	/** 전용 이미지 픽셀 셰이더로 목록의 이미지를 비선행곱 알파 합성한다. 성공과 실패 모두 불투명 기본 상태로 복원하며 빈 목록은 상태 변경 없이 true다. */
	[[nodiscard]] bool DrawUI(const FUIDrawElementList& elements) override;

private:
	/**
	 * 활성 프레임에서 현재 장치의 텍스처를 현재 출력 영역 왼쪽 위 기준 픽셀 위치, 크기로 그린다.
	 * 위치, 크기는 유한하고 크기는 양수여야 하며 행렬 성분의 유한 범위를 넘는 변환은 false다. 음수 위치는 허용하고 출력 영역 밖은 GPU가 자른다.
	 * 월드 변환 데이터는 유지하며 셰이더/버퍼 바인딩은 변경한다. 알파 블렌드 상태는 DrawUI가 설정한다.
	 * true는 그리기 요청 성공이며 GPU 작업 완료나 화면 표시를 뜻하지 않는다.
	 * 텍스처나 SRV가 미준비이거나 D3D11 이외의 렌더 데이터이면 false다. 요소와 텍스처는 호출 동안만 빌린다.
	 * 활성 렌더 타겟과 같은 원본 자원을 읽으려 하면 상수 버퍼 갱신과 그리기 전에 거부한다.
	 */
	[[nodiscard]] bool DrawImage(const FUIImageDrawElement& element);

#pragma endregion

#pragma region RenderResources

private:
	FD3D11Device graphics;

	FD3D11SwapChain swapChain;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

	// 창용 깊이 자원과 별개이며 같은 크기의 캡처에서 재사용한다.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> captureDepthStencilTexture;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> captureDepthStencilView;

	// 깊이 자원의 픽셀 크기이며 0은 아직 준비되지 않았음을 뜻한다.
	int32 captureDepthWidth = 0;

	int32 captureDepthHeight = 0;

	// 출력 중인 자원과 읽기 입력을 비교한다. 렌더 타겟 활성 기간에만 COM 참조를 유지한다.
	Microsoft::WRL::ComPtr<ID3D11Resource> activeRenderTargetResource;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> staticMeshVertexShader;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> imageVertexShader;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> staticMeshPixelShader;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> imagePixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer> transformConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> directionalLightConstantBuffer;

	// VS b2. 세기 반영 RGB와 패딩을 포함한 16바이트이며 초기값은 주변광 기여 0이다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> skyLightConstantBuffer;

	// VS b3. 월드 변환·시점·정반사 RGB·광택 지수를 담는 112바이트이며 메시마다 갱신한다.
	Microsoft::WRL::ComPtr<ID3D11Buffer> specularConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> imageTransformConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> whiteTextureView;

	Microsoft::WRL::ComPtr<ID3D11BlendState> uiBlendState;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> worldDepthStencilState;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> uiDepthStencilState;

	// [U][V] 순서이며 각 축의 0은 Clamp, 1은 Wrap이다. UI는 [0][0]을 사용한다.
	std::array<std::array<Microsoft::WRL::ComPtr<ID3D11SamplerState>, 2>, 2> samplerStates;

	// TODO(seon): UI 배칭, 전용 정점 도입 시 공통 사각형의 보관 형식을 재검토한다.
	TUniquePtr<FStaticMeshRenderData> imageQuad;

	FMatrix4 localToWorld{};

	FMatrix4 worldToClip{};

	// BeginRenderTarget 성공 직전의 행렬이며 EndRenderTarget에서 복원한다.
	FMatrix4 savedLocalToWorld{};

	FMatrix4 savedWorldToClip{};

	int32 bufferWidth = 0;

	int32 bufferHeight = 0;

	// 현재 출력 영역의 픽셀 크기다. 기본 생성 상태는 0이다.
	int32 outputWidth = 0;

	int32 outputHeight = 0;

	bool bInitialized = false;

	bool bFrameActive = false;

	// BeginRenderTarget 성공부터 EndRenderTarget 성공까지 true로 유지한다.
	bool bRenderTargetActive = false;

#pragma endregion
};
