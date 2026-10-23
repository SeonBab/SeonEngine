#pragma once

#include "Object.h"

class FMaterialRenderProxy;

////////////////////////////////////////////////////////////////////////////////
// 메시 자산과 컴포넌트가 공통으로 참조할 재질 계열의 기반 타입이다.
// 구체 재질이 제공하는 렌더 Proxy의 비소유 조회 경계를 정의한다.
////////////////////////////////////////////////////////////////////////////////
class UMaterialInterface : public UObject
{
public:
	/**
	 * 재질이 관리하는 렌더 Proxy의 비소유 주소를 반환한다. 호출자는 삭제하거나 소유권을 가져오지 않는다.
	 * 같은 스레드에서 재질이 살아 있는 동안 사용하며 재질 파괴 후에는 사용하지 않는다.
	 */
	virtual FMaterialRenderProxy* GetRenderProxy() const = 0;
};
