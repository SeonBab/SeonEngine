#pragma once

#include <vector>

class StaticMeshComponent;
class World;
struct SceneRenderData;

/**
 * 한 World의 렌더 데이터를 모아 SceneRenderData로 만든다. 메인 스레드에서 사용하며 World가 소유한다.
 * StaticMeshComponent는 부착 시 여기에 등록하고 소멸 전에 스스로 해제한다.
 */
class RenderScene
{
	// 생성과 소멸
public:
	/** 소유 World를 받는다. World는 이 객체보다 오래 산다. */
	explicit RenderScene(const World& ownerWorld);

	RenderScene(const RenderScene&) = delete;
	RenderScene& operator=(const RenderScene&) = delete;
	RenderScene(RenderScene&&) = delete;
	RenderScene& operator=(RenderScene&&) = delete;

private:
	const World& world;

	// 정적 메시
public:
	/**
	 * 수집 대상으로 추가한다. nullptr이거나 이미 추가한 대상은 무시한다.
	 * 대상은 소멸 전에 RemoveStaticMeshComponent로 제거되어야 한다. 수집 중에는 호출하지 않는다. 할당 실패 예외는 전파한다.
	 */
	void AddStaticMeshComponent(StaticMeshComponent* component);

	/** 수집 대상에서 제거한다. 추가하지 않은 대상은 무시한다. 수집 중에는 호출하지 않는다. */
	void RemoveStaticMeshComponent(StaticMeshComponent* component);

private:
	// 비소유이며 추가한 순서를 유지한다.
	std::vector<StaticMeshComponent*> staticMeshComponents;

	// 수집
public:
	/**
	 * World의 활성 카메라 값과 그릴 영역 크기(픽셀)로 이번 프레임 SceneRenderData를 만든다. World::Tick이 끝난 뒤 호출한다.
	 * 등록된 StaticMeshComponent 중 메시와 TransformComponent가 있는 것만 등록 순서대로 담는다.
	 * 너비나 높이가 0 이하(최소화 등)이거나, 활성 카메라가 없거나, 카메라 값을 얻을 수 없거나,
	 * 투영 행렬을 계산할 수 없으면 출력값을 유지하고 false를 반환한다.
	 */
	bool CollectSceneRenderData(int viewWidth, int viewHeight, SceneRenderData& outSceneRenderData) const;
};
