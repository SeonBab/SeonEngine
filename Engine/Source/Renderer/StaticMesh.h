#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"
#include "Core/Templates/SharedPtr.h"
#include "Core/Templates/UniquePtr.h"
#include "Object.h"

class FStaticMeshRenderData;
class UMaterialInterface;

////////////////////////////////////////////////////////////////////////////////
// 정적 메시 자산의 렌더 데이터를 단독 소유하고 기본 재질들의 수명을 공유한다.
// 기본 상태는 렌더 데이터 미준비이며 조회자는 자산 수명 안에서 데이터를 빌린다.
////////////////////////////////////////////////////////////////////////////////
class UStaticMesh : public UObject
{
public:
	UStaticMesh();
	~UStaticMesh() override;

	UStaticMesh(const UStaticMesh&) = delete;
	UStaticMesh& operator=(const UStaticMesh&) = delete;
	UStaticMesh(UStaticMesh&&) = delete;
	UStaticMesh& operator=(UStaticMesh&&) = delete;

	/**
	 * 소유한 렌더 데이터의 읽기 전용 비소유 포인터를 반환한다. 미준비면 nullptr다.
	 * 자산 파괴 후에는 사용할 수 없으며 렌더 데이터 교체 뒤에는 다시 조회해야 한다.
	 */
	const FStaticMeshRenderData* GetRenderData() const;

	/**
	 * 같은 스레드에서 렌더 데이터의 단독 소유권을 넘겨받는다. 이동 후 입력 포인터는 비며 nullptr는 비움이다.
	 * 기존 데이터는 교체 시 파괴되므로 호출 전에 기존 데이터를 사용하는 모든 비소유 사용을 끝내야 한다.
	 * 이전 조회 주소는 다시 사용하지 않는다. GPU 생성/유효성 검사, Scene 갱신과 재질 변경은 하지 않는다.
	 */
	void SetRenderData(TUniquePtr<FStaticMeshRenderData>&& newRenderData);

	/**
	 * 같은 스레드에서 기본 재질 슬롯 전체를 교체한다. 빈 배열과 nullptr 슬롯도 허용한다.
	 * 공유 참조를 보관하며 GPU 생성이나 Scene 갱신은 수행하지 않는다.
	 */
	void SetStaticMaterials(TArray<TSharedPtr<UMaterialInterface>> materials);

	/**
	 * 같은 스레드에서 슬롯의 공유 참조 사본을 반환한다. 사본은 재질 객체의 수명을 유지한다.
	 * 음수, 범위 밖 또는 미지정 슬롯이면 nullptr다. GPU 준비 상태는 검사하지 않는다.
	 */
	TSharedPtr<UMaterialInterface> GetMaterial(int32 materialIndex) const;

private:
	// 이 자산이 단독 소유한다. nullptr는 렌더 데이터가 아직 준비되지 않은 상태다.
	TUniquePtr<FStaticMeshRenderData> renderData{};

	// 배열 인덱스가 기본 재질 슬롯이다. 빈 배열은 슬롯 없음, nullptr 원소는 재질 미지정이다.
	TArray<TSharedPtr<UMaterialInterface>> staticMaterials;
};
