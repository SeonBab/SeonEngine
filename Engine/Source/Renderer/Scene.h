#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Matrix4.h"
#include "Renderer/StaticMeshRenderData.h"

struct FStaticMeshData;
struct FImage;
class FDirectionalLightSceneProxy;
class FRenderer;

////////////////////////////////////////////////////////////////////////////////
// 등록한 정적 메시의 GPU 자원과 월드 행렬을 소유한다.
// 방향광 Proxy 하나의 주소를 빌리며 소유권은 해당 컴포넌트에 있다.
// 액터, 컴포넌트, 렌더러 주소는 보관하지 않는다.
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
	 * 이미지는 호출 동안만 빌리며 nullptr면 흰색 기본 텍스처를 사용한다.
	 * ID는 이 Scene에서만 유효하며 0은 사용하지 않고 제거 후 재사용하지 않는다. 할당 실패 복구는 제공하지 않는다.
	 */
	// NOTE: 컨벤션 예외: 성공 여부와 성공할 때만 갱신하는 ID 출력을 분리한다.
	[[nodiscard]] bool AddStaticMesh(FRenderer& renderer, const FStaticMeshData& meshData, const FMatrix4& localToWorld, uint64& outMeshId, const FImage* baseColorImage = nullptr);

	/** 등록된 ID의 GPU 메시와 텍스처를 교체한다. 이미지는 호출 동안만 빌리며 nullptr면 흰색이다. ID, 행렬, 순서는 유지하고 실패하면 기존 자원을 보존한다. */
	[[nodiscard]] bool SetMeshData(uint64 meshId, FRenderer& renderer, const FStaticMeshData& meshData, const FImage* baseColorImage = nullptr);

	/** 등록된 ID의 행렬을 복사해 갱신한다. GPU 메시를 다시 만들지 않으며 없는 ID는 false다. */
	[[nodiscard]] bool SetLocalToWorld(uint64 meshId, const FMatrix4& matrix);

	/** 등록 항목과 소유 GPU 참조를 제거한다. 없는 ID는 false이며 다른 ID는 바뀌지 않는다. */
	[[nodiscard]] bool RemoveStaticMesh(uint64 meshId);

	/**
	 * 방향광 Proxy 하나의 주소를 빌려 등록한다. 소유권은 가져오지 않는다.
	 * 이미 등록된 Proxy가 있으면 같은 객체라도 false이며 기존 등록을 유지한다.
	 * Proxy는 등록 해제까지 살아 있어야 하며 같은 스레드에서 사용한다.
	 */
	[[nodiscard]] bool AddDirectionalLight(const FDirectionalLightSceneProxy& proxy);

	/**
	 * 등록된 주소가 인수의 주소와 같을 때 등록을 비운다.
	 * 미등록이거나 다른 주소면 false이며 기존 등록을 유지한다.
	 * Proxy를 파괴하지 않는다. 소유자는 등록 해제 후 객체를 파괴해야 한다.
	 */
	[[nodiscard]] bool RemoveDirectionalLight(const FDirectionalLightSceneProxy& proxy);

	/**
	 * 등록된 Proxy의 비소유 포인터를 반환한다. 미등록이면 nullptr다.
	 * 호출자는 포인터를 장기 보관하지 않으며 등록 해제 후에는 사용하지 않는다.
	 */
	[[nodiscard]] const FDirectionalLightSceneProxy* GetDirectionalLight() const;

	/**
	 * 시작된 프레임 안에서 같은 스레드, 같은 장치의 렌더러로 그린다.
	 * 방향광을 먼저 전달하며 미등록이면 유효한 기본 방향과 RGB 0을 사용한다. 이후 등록 순서대로 메시를 그린다.
	 * 실패하면 false이며 호출자는 반복을 중단하고 Renderer를 Shutdown해야 한다.
	 * 화면 지우기나 EndFrame/Present는 수행하지 않는다.
	 */
	[[nodiscard]] bool Draw(FRenderer& renderer);

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

	// 컴포넌트 소유 Proxy를 빌린다. nullptr는 미등록이며 해제 시 객체를 삭제하지 않는다.
	const FDirectionalLightSceneProxy* directionalLight = nullptr;
};
