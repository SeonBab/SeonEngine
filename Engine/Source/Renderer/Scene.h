#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Math/Matrix4.h"
#include "Core/Templates/UniquePtr.h"

class FDirectionalLightSceneProxy;
class IRenderer;
struct FRenderView;
class FStaticMeshSceneProxy;
class FSkyLightSceneProxy;

////////////////////////////////////////////////////////////////////////////////
// 등록된 정적 메시 Proxy를 단독 소유한다. 빌린 자원의 수명은 등록 요청 측이 유지한다.
// 방향광과 SkyLight Proxy는 컴포넌트에서 빌린다. 메시와 광원의 소유 계약을 구분한다.
// 같은 스레드에서 사용하며 컴포넌트가 등록을 해제할 때까지 Scene이 살아 있어야 한다.
////////////////////////////////////////////////////////////////////////////////
class FScene
{
#pragma region Lifecycle

public:
	FScene();

	~FScene();

	FScene(const FScene&) = delete;

	FScene& operator=(const FScene&) = delete;

	FScene(FScene&&) = delete;

	FScene& operator=(FScene&&) = delete;

#pragma endregion

#pragma region StaticMeshes

public:
	/**
	 * 준비된 Proxy의 단독 소유권을 넘겨받는다. 성공하면 입력은 비고 객체 주소는 유지된다.
	 * null/렌더 데이터 미지정/재질 Proxy 미지정이면 false이며 입력과 목록을 유지한다.
	 * 버퍼 유효성과 장치 일치는 그릴 때 Renderer가 검사한다. 등록만으로 GPU의 전체 유효성을 보장하지 않는다.
	 * 같은 객체의 두 번째 소유 포인터를 만들지 않는다. 자원은 제거 완료까지 유지한다.
	 */
	[[nodiscard]] bool AddStaticMesh(TUniquePtr<FStaticMeshSceneProxy>& proxy);

	/** 기존 주소의 Proxy를 같은 순서에서 교체·파괴한다. 실패 시 기존 상태와 새 입력을 보존한다. */
	[[nodiscard]] bool ReplaceStaticMesh(const FStaticMeshSceneProxy* oldProxy, TUniquePtr<FStaticMeshSceneProxy>& newProxy);

	/** 등록된 주소의 변환만 복사한다. null/미등록은 false며 입력 주소를 역참조하지 않고 찾는다. */
	[[nodiscard]] bool SetLocalToWorld(const FStaticMeshSceneProxy* proxy, const FMatrix4& matrix);

	/** 일치하는 등록을 제거하고 Proxy를 동기 파괴한다. null/미등록은 false다. 제거 후 주소는 무효다. */
	[[nodiscard]] bool RemoveStaticMesh(const FStaticMeshSceneProxy* proxy);

private:
	// 소유 포인터만 이동하므로 배열 재할당에도 등록된 Proxy 객체의 주소는 유지된다.
	TArray<TUniquePtr<FStaticMeshSceneProxy>> staticMeshes;

#pragma endregion

#pragma region DirectionalLight

public:
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

private:
	// 컴포넌트 소유 Proxy를 빌린다. nullptr는 미등록이며 해제 시 객체를 삭제하지 않는다.
	const FDirectionalLightSceneProxy* directionalLight = nullptr;

#pragma endregion

#pragma region SkyLight

public:
	/**
	 * Proxy 주소를 빌려 등록하고 가장 최근 활성 항목으로 선택한다. 같은 주소 중복은 false다.
	 * 객체는 등록 해제까지 살아 있어야 하며 같은 스레드에서 사용한다. 소유권은 가져오지 않는다.
	 */
	[[nodiscard]] bool SetSkyLight(const FSkyLightSceneProxy& proxy);

	/** 해당 주소만 제거한다. 미등록이면 false이며 Proxy는 파괴하지 않는다. 남은 마지막 항목이 활성화된다. */
	[[nodiscard]] bool DisableSkyLight(const FSkyLightSceneProxy& proxy);

	/** 활성 여부와 무관하게 해당 주소가 등록 목록에 있는지 확인한다. */
	[[nodiscard]] bool IsSkyLightRegistered(const FSkyLightSceneProxy& proxy) const;

	/** 마지막 등록 항목의 비소유 주소 또는 nullptr다. 활성 항목 변경/해제 후 다시 조회하며 삭제하지 않는다. */
	[[nodiscard]] const FSkyLightSceneProxy* GetSkyLight() const;

private:
	// 컴포넌트 소유 객체의 주소만 등록 순서대로 빌린다. 마지막 항목 하나만 활성이다.
	TArray<const FSkyLightSceneProxy*> skyLightStack;

#pragma endregion

#pragma region Rendering

public:
	/**
	 * 시작된 프레임 안에서 같은 스레드, 같은 장치의 렌더러로 그린다.
	 * 시점, 광원과 등록 순서의 메시 요청을 구성해 DrawScene에 한 번 전달한다. 미등록 광원의 기여는 RGB 0이다.
	 * 방향광의 미등록 방향은 유효한 기본값을 사용한다. SkyLight는 현재 균일 주변광으로 적용한다.
	 * 요청 처리 동안 관련 자원 교체·파괴나 재진입 변경을 하지 않는다. Scene과 Proxy의 소유 관계는 유지한다.
	 * 실패하면 false이며 호출자는 반복을 중단하고 Renderer를 Shutdown해야 한다.
	 * 화면 지우기나 EndFrame/Present는 수행하지 않는다.
	 */
	[[nodiscard]] bool Draw(IRenderer& renderer, const FRenderView& view);

#pragma endregion
};
