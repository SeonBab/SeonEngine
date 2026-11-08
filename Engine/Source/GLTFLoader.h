#pragma once

#include <filesystem>

struct StaticMeshData;

/** glTF 파일을 엔진 데이터로 불러온다. */
namespace GLTFLoader
{
	/** .gltf 파일에서 첫 메시의 첫 프리미티브를 읽는다. 읽지 못하면 출력값을 유지하고 false를 반환한다. */
	bool LoadStaticMesh(const std::filesystem::path& path, StaticMeshData& outMeshData);
}
