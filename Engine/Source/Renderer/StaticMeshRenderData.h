#pragma once

////////////////////////////////////////////////////////////////////////////////
// 렌더러별 정적 메시 GPU 데이터를 공통 포인터로 소유하기 위한 기반이다.
// 실제 자원은 파생 타입이 소유하며 생성한 렌더러에서만 사용한다.
// 객체 자체는 복사하거나 이동하지 않고 단독 소유 포인터를 이동해 소유권을 전달한다.
////////////////////////////////////////////////////////////////////////////////
class FStaticMeshRenderData
{
public:
	/** 기반 포인터로 파괴해도 실제 렌더러 데이터의 자원을 해제한다. */
	virtual ~FStaticMeshRenderData();

	FStaticMeshRenderData(const FStaticMeshRenderData&) = delete;
	FStaticMeshRenderData& operator=(const FStaticMeshRenderData&) = delete;
	FStaticMeshRenderData(FStaticMeshRenderData&&) = delete;
	FStaticMeshRenderData& operator=(FStaticMeshRenderData&&) = delete;

protected:
	FStaticMeshRenderData() = default;
};
