#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Matrix4.h"
#include "Renderer/DirectionalLightData.h"
#include "Renderer/StaticMeshRenderData.h"

#include <optional>

class FRenderer;
struct FStaticMeshData;
struct FImage;

////////////////////////////////////////////////////////////////////////////////
// 등록한 정적 메시의 GPU 자원, 월드 행렬과 방향광 하나의 CPU 데이터를 소유한다.
// 액터, 컴포넌트, 렌더러 주소는 보관하지 않으며 변경 전달은 호출자가 맡는다.
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
	 * 방향광 데이터를 복사해 하나만 등록한다. Renderer나 GPU 자원은 사용하지 않는다.
	 * 이미 등록됐거나 ID가 소진되면 false이며 기존 데이터와 출력 ID를 유지한다.
	 * ID는 이 Scene의 방향광 등록에만 유효하고 0은 사용하지 않으며 재사용하지 않는다.
	 * 입력은 FDirectionalLightData의 계약을 따르며 값 검사나 정규화는 하지 않는다.
	 */
	// NOTE: 컨벤션 예외: 성공 여부와 성공할 때만 갱신하는 ID 출력을 분리한다.
	[[nodiscard]] bool AddDirectionalLight(const FDirectionalLightData& data, uint64& outLightId);

	/**
	 * 일치하는 ID로 등록된 방향광 데이터를 복사해 교체한다.
	 * 미등록 또는 다른 ID면 false이고 기존 값을 유지한다. 성공해도 등록 ID는 바뀌지 않는다.
	 * 입력은 FDirectionalLightData의 계약을 따르며 값 검사, 정규화, GPU 갱신은 하지 않는다.
	 */
	[[nodiscard]] bool SetDirectionalLight(uint64 lightId, const FDirectionalLightData& data);

	/**
	 * 일치하는 ID의 방향광 등록을 제거한다. 미등록 또는 다른 ID면 false이며 기존 상태를 유지한다.
	 * 이전 조회 포인터는 무효가 된다. 다음 ID는 유지해 재등록 시 제거한 ID를 재사용하지 않는다.
	 * 호출자가 보관한 ID 변수는 변경하지 않으며 GPU 갱신은 수행하지 않는다.
	 */
	[[nodiscard]] bool RemoveDirectionalLight(uint64 lightId);

	/**
	 * 등록된 방향광 데이터를 읽기 전용으로 빌린다. 미등록이면 nullptr이며 세기 0도 등록된 데이터로 반환한다.
	 * 소유권은 Scene에 있고 Scene 파괴 후 포인터는 무효다. 보관 값의 갱신, 제거 뒤에는 다시 조회한다.
	 */
	[[nodiscard]] const FDirectionalLightData* GetDirectionalLight() const;

	/**
	 * 시작된 프레임 안에서 같은 스레드, 같은 장치의 렌더러로 그린다.
	 * 방향광을 먼저 전달하며 미등록이면 세기 0을 사용한다. 이후 등록 순서대로 메시를 그린다.
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

	struct FDirectionalLightEntry
	{
		uint64 lightId = 0;
		FDirectionalLightData data;
	};

	FStaticMeshEntry* FindMeshEntry(uint64 meshId);

	TArray<FStaticMeshEntry> staticMeshes;
	uint64 nextMeshId = 1;
	std::optional<FDirectionalLightEntry> directionalLight;
	uint64 nextDirectionalLightId = 1;
};
