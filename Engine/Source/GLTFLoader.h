#pragma once

#include "Core/Math/Transform.h"
#include "Mesh/StaticMesh.h"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

/**
 * glTF 노드 하나를 엔진 좌표계(왼손, cm)로 바꾼 배치 정보다.
 * 배치표의 번호는 glTF nodes 번호와 같다.
 */
struct GLTFSceneNode
{
	// 부모 노드의 배치표 번호. 없으면 최상위 노드다.
	std::optional<std::size_t> parent;
	// 자식 노드들의 배치표 번호. 부모 번호와 같은 관계를 반대 방향으로 담는다.
	std::vector<std::size_t> children;
	// 그릴 glTF meshes 번호. 없으면 메시 없이 위치만 갖는 노드다.
	std::optional<std::size_t> mesh;
	// 부모 기준 위치, 회전, 크기. 최상위 노드는 파일 원점 기준이다.
	Transform localTransform;
	// 사람이 보기 위한 이름
	std::string name;
};

/**
 * glTF 파일 하나에서 얻은 메시들과 배치표다. 모두 엔진 좌표계(왼손, cm)다.
 * meshes는 glTF meshes 번호, sceneNodes는 glTF nodes 번호와 같은 순서라, 배치표의 mesh 번호로 meshes를 찾는다.
 * meshes의 경로는 비어 있으며, 받는 쪽이 에셋으로 보관할 때 정한다.
 */
struct GLTFModel
{
	std::vector<StaticMesh> meshes;
	std::vector<GLTFSceneNode> sceneNodes;
};

/** glTF 파일을 엔진 데이터로 불러온다. */
namespace GLTFLoader
{
	/** .gltf 파일의 모든 메시와 배치표를 채운다. 메시마다 첫 프리미티브만 쓴다. 실패하면 출력값을 유지하고 false를 반환한다. */
	bool LoadModel(const std::filesystem::path& path, GLTFModel& outModel);
}
