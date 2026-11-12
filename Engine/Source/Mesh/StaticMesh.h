#pragma once

#include "Asset.h"
#include "Mesh/MeshVertex.h"

#include <cstdint>
#include <vector>

/**
 * 이름과 경로 및 CPU 정점과 인덱스 배열을 소유하는 정적 메시 에셋이다. 인덱스 3개가 삼각형 하나이며,
 * 바깥에서 볼 때 시계 방향으로 감긴 면이 앞면이다. GPU 자원은 소유하지 않는다.
 */
class StaticMesh : public Asset
{
public:
	/**
	 * UTF-8 표시 이름과 저장소 루트 기준 상대 경로를 받는다. 파일 없이 생성하면 빈 경로를 사용한다.
	 * 파일 안의 여러 메시 중 하나면 경로 뒤에 "#번호"가 붙는다.
	 */
	StaticMesh(std::string newName, std::string newPath);

public:
	std::vector<MeshVertex> vertices;
	// 각 값은 vertices의 요소 번호다.
	std::vector<std::uint32_t> indices;
};
