#pragma once

#include "Core/Math/Transform.h"
#include "Core/Math/Vector3.h"
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
 * glTF 재질 하나를 엔진에서 쓸 값으로 바꾼 설명이다. 텍스처는 아직 불러오지 않고 이미지 경로만 담는다.
 * 받는 쪽이 이 설명으로 Material을 만든다. 계수의 의미와 범위는 Material과 같으므로 Material이 바뀌면 함께 맞춘다.
 */
struct GLTFModelMaterial
{
	// 색 텍스처 값에 곱할 선형 RGB 계수. 각 성분은 0 이상 1 이하다. glTF의 알파 계수는 아직 쓰지 않는다.
	Vector3 baseColor = { 1.0f, 1.0f, 1.0f };
	// 색 텍스처 이미지의 .gltf 폴더 기준 UTF-8 상대 경로. 비어 있으면 색 텍스처가 없다.
	// glTF uri의 퍼센트 인코딩(%20 등)은 풀지 않은 그대로다.
	std::string baseColorImageUri;
	// 금속성 계수. 0 이상 1 이하다.
	float metallic = 1.0f;
	// 거칠기 계수. 0 이상 1 이하다.
	float roughness = 1.0f;
	// 사람이 보기 위한 이름
	std::string name;
};

/**
 * glTF 파일 하나에서 얻은 메시들과 재질들과 배치표다. 모두 엔진 좌표계(왼손, cm)다.
 * meshes는 glTF meshes 번호, sceneNodes는 glTF nodes 번호와 같은 순서라, 배치표의 mesh 번호로 meshes를 찾는다.
 * materials는 glTF materials 번호 순서이며, meshMaterials의 번호로 찾는다.
 * meshes의 경로는 비어 있으며, 받는 쪽이 에셋으로 보관할 때 정한다.
 */
struct GLTFModel
{
	std::vector<StaticMesh> meshes;
	std::vector<GLTFModelMaterial> materials;
	// meshes와 같은 순서이며, 각 메시 첫 프리미티브의 materials 번호다. 없으면 glTF 기본 재질을 쓴다는 뜻이다.
	std::vector<std::optional<std::size_t>> meshMaterials;
	std::vector<GLTFSceneNode> sceneNodes;
};

/** glTF 파일을 엔진 데이터로 불러온다. */
namespace GLTFLoader
{
	/**
	 * .gltf 파일의 모든 메시, 재질과 배치표를 채운다. 메시마다 첫 프리미티브만 쓴다.
	 * 재질은 색, 금속성, 거칠기와 색 텍스처 경로만 읽는다. 실패하면 출력값을 유지하고 false를 반환한다.
	 */
	bool LoadModel(const std::filesystem::path& path, GLTFModel& outModel);
}
