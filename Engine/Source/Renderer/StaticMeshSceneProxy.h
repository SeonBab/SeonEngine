#pragma once

#include "Core/Math/Matrix4.h"

struct FStaticMeshSceneProxyDesc;
class FStaticMeshRenderData;
class FMaterialRenderProxy;
class UMaterialInterface;

////////////////////////////////////////////////////////////////////////////////
// 정적 메시의 비소유 형상/재질 참조와 인스턴스 변환을 렌더 상태로 보관한다.
// 자원은 소유 측에서 사용 종료까지 유지한다. 생성, 조회, 변경과 파괴는 같은 스레드에서 수행한다.
////////////////////////////////////////////////////////////////////////////////
class FStaticMeshSceneProxy
{
#pragma region Lifecycle

public:
	/**
	 * Desc에서 형상 주소를 빌리고 선택 재질과 행렬을 저장한다. 재질 미지정이면 공통 기본 재질을 사용한다.
	 * Desc 자체는 보관하지 않는다. 생성만으로 GPU 준비/유효성 검사나 Scene 등록은 수행하지 않는다.
	 */
	explicit FStaticMeshSceneProxy(const FStaticMeshSceneProxyDesc& desc);

	// 등록 시 사용하는 객체 주소와 정체성을 유지한다. 소유 포인터의 이동과 객체 자체의 이동은 다르다.
	FStaticMeshSceneProxy(const FStaticMeshSceneProxy&) = delete;

	FStaticMeshSceneProxy& operator=(const FStaticMeshSceneProxy&) = delete;

	FStaticMeshSceneProxy(FStaticMeshSceneProxy&&) = delete;

	FStaticMeshSceneProxy& operator=(FStaticMeshSceneProxy&&) = delete;

#pragma endregion

#pragma region Resources

public:
	/**
	 * 생성 때 얻은 형상 주소를 빌린다. 자산 미지정/미준비면 nullptr이며 nonnull이 GPU 유효성을 보장하지 않는다.
	 * 자산의 렌더 데이터 교체/비움/파괴 전에 이 Proxy의 사용을 끝내야 한다.
	 */
	const FStaticMeshRenderData* GetRenderData() const;

	/** 재질이 제공하는 Proxy의 비소유 주소를 반환한다. 재질/모듈 수명 안에서만 사용하며 삭제하지 않는다. */
	FMaterialRenderProxy* GetMaterialRenderProxy() const;

private:
	// 생성 때 조회한 비소유 주소다. nullptr는 형상 미준비이며 이후 자산을 다시 조회하지 않는다.
	const FStaticMeshRenderData* renderData;

	// 선택 재질 또는 공통 기본 재질의 비소유 주소다. 생성 후에는 nullptr가 아니다.
	const UMaterialInterface* material;

#pragma endregion

#pragma region Transform

public:
	/** 로컬에서 월드로의 변환을 값으로 반환한다. 원본 상태는 바뀌지 않는다. */
	FMatrix4 GetLocalToWorld() const;

	/** 변환 사본만 교체한다. 형상 재생성, GPU 업로드나 Scene 갱신은 수행하지 않는다. */
	void SetLocalToWorld(const FMatrix4& matrix);

private:
	// 외부 입력/반환 행렬과 독립된 인스턴스 변환 사본이다.
	FMatrix4 localToWorld;

#pragma endregion
};
