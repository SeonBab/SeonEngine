#pragma once

#include "Containers/Array.h"
#include "CoreTypes.h"
#include "Math/Matrix4.h"
#include "Renderer/StaticMeshRenderData.h"

class FRenderer;
struct FStaticMeshData;
struct FModelMeshData;
struct FImage;

////////////////////////////////////////////////////////////////////////////////
// 등록한 정적 메시의 GPU 자원과 월드 행렬을 소유한다.
// 액터·컴포넌트·렌더러 주소는 보관하지 않으며 변경 전달은 호출자가 맡는다.
////////////////////////////////////////////////////////////////////////////////
class FScene
{
public:
	FScene() = default;

	FScene(const FScene&) = delete;
	FScene& operator=(const FScene&) = delete;
	FScene(FScene&&) = delete;
	FScene& operator=(FScene&&) = delete;

	/**
	 * 준비된 렌더러로 GPU 메시를 만들고 행렬을 복사해 등록한다. 실패하면 기존 목록과 출력 ID를 유지한다.
	 * ID는 이 Scene에서만 유효하며 0은 사용하지 않고 제거 후 재사용하지 않는다. 할당 실패 복구는 제공하지 않는다.
	 */
	// NOTE: 컨벤션 예외 — 성공 여부와 성공할 때만 갱신하는 ID 출력을 분리한다.
	[[nodiscard]] bool AddStaticMesh(FRenderer& renderer, const FStaticMeshData& meshData, const FMatrix4& localToWorld, uint64& outMeshId);

	/** 같은 등록 계약으로 모델 메시를 직접 업로드한다. 현재 재질·노드는 사용하지 않고 모델용 흰색 경로로 그린다. */
	[[nodiscard]] bool AddStaticMesh(FRenderer& renderer, const FModelMeshData& meshData, const FMatrix4& localToWorld, uint64& outMeshId);

	/** 같은 등록 계약으로 모델 메시와 디코딩한 RGBA8 이미지를 업로드한다. CPU 입력과 파일 경로는 보관하지 않는다. */
	[[nodiscard]] bool AddStaticMesh(FRenderer& renderer, const FModelMeshData& meshData, const FImage& baseColorImage, const FMatrix4& localToWorld, uint64& outMeshId);

	/** 등록된 ID의 GPU 메시만 교체한다. ID·행렬·순서는 유지하며 없는 ID나 생성 실패는 false이고 기존 자원을 보존한다. */
	[[nodiscard]] bool SetMeshData(uint64 meshId, FRenderer& renderer, const FStaticMeshData& meshData);

	/** 같은 교체 계약으로 모델 메시를 설정한다. 기존 정점 형식과 달라도 ID·행렬·순서를 유지하고 실패하면 기존 자원을 보존한다. */
	[[nodiscard]] bool SetMeshData(uint64 meshId, FRenderer& renderer, const FModelMeshData& meshData);

	/** 모델 메시와 이미지를 함께 교체한다. ID·행렬·순서는 유지하며 실패하면 기존 버퍼와 텍스처를 보존한다. */
	[[nodiscard]] bool SetMeshData(uint64 meshId, FRenderer& renderer, const FModelMeshData& meshData, const FImage& baseColorImage);

	/** 등록된 ID의 행렬을 복사해 갱신한다. GPU 메시를 다시 만들지 않으며 없는 ID는 false다. */
	[[nodiscard]] bool SetLocalToWorld(uint64 meshId, const FMatrix4& matrix);

	/** 등록 항목과 소유 GPU 참조를 제거한다. 없는 ID는 false이며 다른 ID는 바뀌지 않는다. */
	[[nodiscard]] bool RemoveStaticMesh(uint64 meshId);

	/**
	 * 같은 장치의 렌더러로 BeginFrame→등록 순서의 행렬 설정/그리기→EndFrame을 수행한다. 빈 Scene도 화면을 지운다.
	 * 같은 스레드의 프레임 사이에 호출하며 실패하면 false다. 그리기 전/도중 실패는 Present하지 않는다.
	 * NOTE: 실패 시 반복을 중단하고 Renderer를 Shutdown해야 한다. 진행 중인 프레임을 자동으로 닫거나 복구하지 않는다.
	 */
	[[nodiscard]] bool RenderFrame(FRenderer& renderer, int width, int height);

private:
	struct FStaticMeshEntry
	{
		uint64 meshId = 0;
		FStaticMeshRenderData renderData;
		FMatrix4 localToWorld{};
	};

	FStaticMeshEntry* FindMeshEntry(uint64 meshId);

	TArray<FStaticMeshEntry> staticMeshes;
	uint64 nextMeshId = 1;
};
