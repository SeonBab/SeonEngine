#pragma once

#include "Component.h"

#include <memory>

class RenderScene;
struct StaticMeshData;

/**
 * 그릴 정적 메시를 공유 참조로 보관한다. 위치, 회전, 크기는 같은 객체의 TransformComponent를 따른다.
 * 부착 시 소속 World의 RenderScene에 추가되고 소멸 시 제거된다.
 */
class StaticMeshComponent : public Component
{
	// 생성과 소멸
public:
	StaticMeshComponent() = default;

	/** 추가된 RenderScene이 있으면 그곳에서 이 컴포넌트를 제거한다. */
	~StaticMeshComponent() override;

	// 등록
public:
	/** 소속 World의 RenderScene에 이 컴포넌트를 추가한다. GameObject::AddComponent가 호출한다. */
	void OnRegister() override;

private:
	// 소멸 시 GameObject를 거치지 않고 제거하려고 보관한다. 비소유이며 World가 이 컴포넌트보다 오래 산다.
	RenderScene* registeredRenderScene = nullptr;

	// 메시
public:
	/** 그릴 메시를 지정한다. nullptr는 그리지 않음이다. 여러 컴포넌트가 공유하므로 지정한 메시 데이터는 이후 바꾸지 않는다. */
	void SetStaticMesh(std::shared_ptr<const StaticMeshData> newStaticMesh);

	/** 지정한 메시의 공유 참조를 빌려준다. 지정하지 않았으면 비어 있다. */
	const std::shared_ptr<const StaticMeshData>& GetStaticMesh() const { return staticMesh; }

private:
	std::shared_ptr<const StaticMeshData> staticMesh;
};
