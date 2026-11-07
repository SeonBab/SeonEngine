#include "RenderScene.h"

#include "CameraViewInfo.h"
#include "Component/CameraComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/TransformComponent.h"
#include "Core/Math/Transform.h"
#include "GameObject.h"
#include "Renderer/SceneRenderData.h"
#include "World.h"

#include <algorithm>
#include <utility>

RenderScene::RenderScene(const World& ownerWorld)
	: world(ownerWorld)
{
}

void RenderScene::AddStaticMeshComponent(StaticMeshComponent* component)
{
	if (component == nullptr) { return; }
	if (std::find(staticMeshComponents.begin(), staticMeshComponents.end(), component) != staticMeshComponents.end()) { return; }

	staticMeshComponents.push_back(component);
}

void RenderScene::RemoveStaticMeshComponent(StaticMeshComponent* component)
{
	const auto found = std::find(staticMeshComponents.begin(), staticMeshComponents.end(), component);
	if (found == staticMeshComponents.end()) { return; }

	// 그리는 순서가 의미를 가질 수 있으므로 마지막 원소와 바꾸지 않고 순서를 유지해 지운다.
	staticMeshComponents.erase(found);
}

bool RenderScene::CollectSceneRenderData(int viewWidth, int viewHeight, SceneRenderData& outSceneRenderData) const
{
	// 0으로 나누면 화면 비율이 무한대나 NaN이 되어 투영 검사를 통과할 수 있으므로 나누기 전에 거른다.
	if (viewWidth <= 0 || viewHeight <= 0) { return false; }
	const float aspectRatio = static_cast<float>(viewWidth) / static_cast<float>(viewHeight);

	const CameraComponent* camera = world.GetActiveCamera();
	if (camera == nullptr) { return false; }

	CameraViewInfo cameraViewInfo;
	if (!camera->GetCameraViewInfo(cameraViewInfo)) { return false; }

	// 중간에 실패해도 출력이 일부만 채워진 채 남지 않도록 지역 값에 먼저 만든다.
	SceneRenderData sceneRenderData;
	if (!cameraViewInfo.CalculateProjectionMatrix(aspectRatio, sceneRenderData.view.projectionMatrix)) { return false; }
	sceneRenderData.view.viewMatrix = cameraViewInfo.CalculateViewMatrix();
	sceneRenderData.view.cameraPosition = cameraViewInfo.position;

	sceneRenderData.staticMeshes.reserve(staticMeshComponents.size());
	for (const StaticMeshComponent* component : staticMeshComponents)
	{
		// 메시나 TransformComponent가 없으면 이 항목만 건너뛴다. 카메라와 달리 전체 실패로 보지 않는다.
		const std::shared_ptr<const StaticMeshData>& mesh = component->GetStaticMesh();
		if (mesh == nullptr) { continue; }

		// 등록은 OnRegister에서 소속 객체가 있을 때만 하므로 GetGameObject는 nullptr가 아니다.
		const TransformComponent* transform = component->GetGameObject()->GetComponent<TransformComponent>();
		if (transform == nullptr) { continue; }

		sceneRenderData.staticMeshes.push_back(StaticMeshDrawItem{ mesh, transform->GetTransform().ToMatrix() });
	}

	// 배열을 복사하지 않도록 옮긴다.
	outSceneRenderData = std::move(sceneRenderData);
	return true;
}
