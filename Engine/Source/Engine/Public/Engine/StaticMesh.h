#pragma once

#include "Containers/Array.h"
#include "Renderer/PrimitiveType.h"
#include "Renderer/StaticMeshVertex.h"
#include "UObject/Object.h"

////////////////////////////////////////////////////////////////////////////////
// 여러 컴포넌트가 사용할 스태틱 메시 자산이다.
// 로컬 위치 / 색상 / UV과 정점 연결 규칙을 보관하고 컴포넌트가 렌더 데이터로 복사한다. GPU 자원은 보관하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class UStaticMesh : public UObject
{
public:
	/**
	 * 소속과 빈 CPU 형상을 준비한다. GPU 자원 준비는 하지 않는다.
	 * @param inOuter nullptr이면 소속이 없다. 소속 조회 / 사용 동안 대상이 살아 있어야 한다.
	 */
	explicit UStaticMesh(UObject* inOuter = nullptr);

	/** 메시 객체와 CPU 형상을 파괴한다. Outer를 파괴하거나 GPU 자원을 정리하지 않는다. */
	~UStaticMesh() override = default;

	/**
	 * 정점과 연결 규칙을 검사한 뒤 CPU 형상을 복사하여 교체한다. GPU 갱신은 하지 않는다.
	 * @param inVertices 빈 배열은 형상을 비운다. List는 3의 배수, 빈 배열이 아닌 Strip은 3개 이상이어야 한다.
	 * @param inPrimitiveType 지원하는 Triangle List / Strip 규칙이다.
	 * @return 설정하면 true. 지원하지 않는 규칙이나 맞지 않는 정점 수는 false이며 기존 형상을 유지한다.
	 */
	[[nodiscard]] bool SetMeshData(const TArray<FStaticMeshVertex>& inVertices, EPrimitiveType inPrimitiveType);

	/** 정점 배열을 읽기 전용으로 빌린다. 메시 파괴 후 무효이며 재설정 뒤 원소 참조는 다시 얻는다. */
	const TArray<FStaticMeshVertex>& GetVertices() const;

	/** 저장한 정점 연결 규칙을 반환한다. 기본값은 Triangle List다. */
	EPrimitiveType GetPrimitiveType() const;

private:
	TArray<FStaticMeshVertex> vertices;
	EPrimitiveType primitiveType = EPrimitiveType::PT_TriangleList;
};
