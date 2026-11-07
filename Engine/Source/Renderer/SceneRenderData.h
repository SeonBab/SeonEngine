#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Renderer/RenderView.h"

#include <memory>
#include <vector>

struct StaticMeshData;

/** 한 프레임에 그릴 정적 메시 하나의 요청이다. */
struct StaticMeshDrawItem
{
	// 공유 참조라서 렌더러가 쓰는 동안 메시가 사라지지 않는다. 같은 주소는 같은 형상이므로 GPU 자원을 재사용할 수 있다.
	std::shared_ptr<const StaticMeshData> mesh;
	// 메시 로컬 좌표를 월드 좌표로 옮기는 행렬이다. 크기, 회전, 이동 순서이며 행 벡터 규칙을 따른다.
	Matrix4x4 worldMatrix;
};

/**
 * 한 프레임에 렌더러로 넘길 장면 정보의 값 묶음이다. 매 프레임 World에서 새로 모은다.
 * 렌더러는 이 묶음만 보고 GameObject와 컴포넌트를 직접 읽지 않는다.
 */
struct SceneRenderData
{
	RenderView view;
	// 등록 순서대로 담는다.
	std::vector<StaticMeshDrawItem> staticMeshes;
};
