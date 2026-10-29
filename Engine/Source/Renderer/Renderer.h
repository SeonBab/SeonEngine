#pragma once

#include "Core/CoreTypes.h"
#include "Core/Templates/UniquePtr.h"

#include <span>

struct FImage;
struct FSceneRenderData;
struct FStaticMeshData;
class FStaticMeshRenderData;
class FUIDrawElementList;
class IImGuiRendererBackend;
class UFont;
class UTexture2D;
class UTextureRenderTarget2D;

/**
 * 자원 생성·장면 및 UI 제출·출력과 프레임의 공통 계약이다. 생성한 구현과 장치의 자원만 사용한다.
 * 초기화·사용·정리는 같은 스레드에서 수행한다. 창과 이 객체의 소유는 호출자가 맡고 종료 전 Shutdown을 호출한다.
 * 미지원 요청은 실패로 반환하며 다른 구현으로 자동 대체하지 않는다. 성공도 GPU 완료나 화면 표시를 보장하지 않는다.
 */
class IRenderer
{
public:
	virtual ~IRenderer() = default;
	IRenderer(const IRenderer&) = delete;
	IRenderer& operator=(const IRenderer&) = delete;
	IRenderer(IRenderer&&) = delete;
	IRenderer& operator=(IRenderer&&) = delete;

	/** 구현에 맞는 플랫폼 창 포인터와 양수 실제 클라이언트 크기로 준비한다. 창은 Shutdown까지 유지한다. 중복 초기화는 거부한다. */
	[[nodiscard]] virtual bool Initialize(void* nativeWindowHandle, int32 width, int32 height) = 0;

	/** UI backend·장면 사용을 끝낸 뒤 구현의 남은 요청과 소유 자원을 정리한다. 빈 상태·부분 실패·반복 호출을 허용한다. */
	virtual void Shutdown() = 0;

	/**
	 * 초기화 후 CPU 삼각형 목록을 복사해 형상을 만든다. 인덱스는 정점 범위 안이고 개수는 3의 배수여야 한다.
	 * 성공하면 출력 객체를 교체하고 실패하면 이전 출력을 유지한다. 교체 전 기존 비소유 사용을 끝내야 한다.
	 * 호출 후 CPU 입력은 보관할 필요가 없다. 재질 번호와 CPU 주소 설정은 사용하지 않는다.
	 */
	[[nodiscard]] virtual bool CreateMesh(const FStaticMeshData& meshData, TUniquePtr<FStaticMeshRenderData>& outMesh) = 0;

	/** 양수 크기의 RGBA8 CPU 이미지로 전체 밉 텍스처를 준비한다. 성공 시 데이터·크기만 교체하고 실패 시 기존 출력을 유지한다. 주소 설정과 자산 주소는 유지한다. */
	[[nodiscard]] virtual bool CreateTexture2D(const FImage& image, UTexture2D& outTexture) = 0;

	/**
	 * 원본 DDS 바이트로 단일 2D 색상 텍스처를 만든다. 입력은 호출 동안만 빌리고 파일을 변경하지 않는다.
	 * RGBA/BGRA/BGRX 8비트 UNORM과 BC1/BC2/BC3의 UNORM 및 sRGB 형식을 지원하며 원본 밉을 유지한다.
	 * 배열, 큐브, 볼륨과 Custom 알파 표현은 거부한다. 미지정 알파 표현은 Straight로 간주한다.
	 * 생성된 자원의 실제 크기와 알파 표현을 저장하며 원본 픽셀 변환이나 새 밉 생성은 하지 않는다.
	 * 초기화된 Renderer의 같은 스레드에서 호출한다. 성공 시에만 출력 데이터를 교체하며 실패 시 기존 출력과 주소 설정을 유지한다.
	 * 성공 교체 전 기존 데이터의 비소유 사용을 끝낸다. true는 GPU 완료를 뜻하지 않는다.
	 */
	[[nodiscard]] virtual bool CreateTexture2DFromDDS(std::span<const uint8> ddsBytes, UTexture2D& outTexture) = 0;

	/**
	 * 초기화 후 구현이 지원하는 컴파일된 폰트 바이트로 자원을 생성한다. 입력은 호출 동안만 빌린다.
	 * 아틀라스 픽셀은 Premultiplied 표현이어야 한다. 파일에서 표현을 자동 판별하거나 픽셀을 변환하지 않는다.
	 * 성공 시에만 출력을 교체한다. 교체 전 기존 데이터의 비소유 사용을 끝내야 하며 입력은 반환 후 해제할 수 있다.
	 * 미초기화와 빈 입력은 로그와 false로 전달한다. 외부 라이브러리의 형식 및 자원 생성 예외는 잡지 않고 전파한다.
	 * false 반환이나 생성 중 예외는 이전 출력을 유지한다. 상위 실행 복구와 GPU 완료를 보장하지 않는다.
	 */
	[[nodiscard]] virtual bool CreateFontResource(std::span<const uint8> fontData, UFont& outFont) = 0;

	/** 활성 프레임 밖에서 양수 크기의 RGBA8 UNORM 단일 밉·샘플 색상 타겟을 검정/알파1로 준비한다. 성공 시 데이터·크기를 함께 교체하고 실패 시 이전 출력을 유지한다. */
	[[nodiscard]] virtual bool CreateRenderTarget2D(int32 width, int32 height, UTextureRenderTarget2D& outTexture) = 0;

	/** 초기화 후 양수 실제 크기로 프레임을 시작하고 창 출력을 준비·지운다. 이미 활성인 프레임은 거부한다. */
	[[nodiscard]] virtual bool BeginFrame(int32 width, int32 height) = 0;

	/** 활성 프레임에서 준비된 타겟 출력으로 전환하고 색상·깊이를 지운다. 중첩은 거부하며 실패 시 기존 출력·행렬을 유지한다. */
	[[nodiscard]] virtual bool BeginRenderTarget(const UTextureRenderTarget2D& target) = 0;

	/** 활성 타겟을 끝내고 창 출력·기존 행렬을 복원한다. clear·Present는 하지 않는다. 실패하면 활성 상태를 유지한다. */
	[[nodiscard]] virtual bool EndRenderTarget() = 0;

	/**
	 * 활성 프레임에 한 시점의 장면을 제출한다. 출력 선택·clear·Present는 하지 않는다. 실패 전 제출을 롤백하지 않는다.
	 * 호출 중 자원 교체·파괴와 재진입 변경은 금지한다. 지연 구현은 반환 전에 CPU 값과 필요한 GPU 자원 수명을 확보한다.
	 * 요청 배열·Proxy·빌린 형상 주소를 반환 후 사용하려고 저장하면 안 된다. 미준비 텍스처는 흰색을 사용한다.
	 */
	[[nodiscard]] virtual bool DrawScene(const FSceneRenderData& data) = 0;

	/**
	 * 활성 프레임에 이미지 목록을 자원의 Straight/Premultiplied 표현에 맞춰 제출한다. 빈 목록은 상태 변경 없이 성공한다.
	 * 입력은 호출 동안만 빌리며 자원과 메타데이터를 교체/파괴하지 않는다. 지연 구현은 값과 GPU 수명을 반환 전에 확보한다.
	 * 목록 전체의 준비/자원 검증 실패는 이번 호출을 그리지 않는다. 제출 중 실패는 이후 배치를 중단하며 이미 제출한 결과는 롤백하지 않는다.
	 * true는 제출 요청 성공이며 GPU 완료나 화면 표시를 뜻하지 않는다. 출력 선택·clear·Present는 수행하지 않는다.
	 */
	[[nodiscard]] virtual bool DrawUI(const FUIDrawElementList& elements) = 0;

	/** 프레임을 닫고 표시를 요청한다. 타겟 활성 중에는 상태를 유지하고 거부한다. 표시 요청 실패 시에도 프레임은 이미 종료됐을 수 있다. */
	[[nodiscard]] virtual bool EndFrame() = 0;

	/** 초기화된 구현에 맞는 미초기화 ImGui backend를 만든다. 미지원/미초기화는 nullptr다. 반환 객체는 이 Renderer보다 먼저 정리해야 한다. */
	[[nodiscard]] virtual TUniquePtr<IImGuiRendererBackend> CreateImGuiRendererBackend() = 0;

protected:
	IRenderer() = default;
};
