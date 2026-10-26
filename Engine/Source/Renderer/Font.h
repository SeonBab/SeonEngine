#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Object.h"

class FFontRenderData;

/** 렌더러별 폰트 데이터를 단독 소유하는 자산이다. 로딩과 출력 기능은 제공하지 않는다. */
class UFont : public UObject
{
public:
	UFont();
	~UFont() override;

	/** 미지정이면 nullptr다. 반환 주소는 자산 수명 안에서만 빌리며 데이터 교체나 비움 뒤에는 다시 조회해야 한다. */
	const FFontRenderData* GetRenderData() const;

	/**
	 * 같은 스레드에서 렌더 데이터의 단독 소유권을 넘겨받는다. 이동 후 입력은 비며 nullptr는 비움이다.
	 * 교체 전 기존 데이터의 비소유 사용을 끝내고 이전 조회 주소는 재사용하지 않는다.
	 * GPU 생성, 유효성 검사, 구현 호환성 검사와 GPU 완료 대기는 수행하지 않는다.
	 */
	void SetRenderData(TUniquePtr<FFontRenderData>&& newRenderData);

private:
	TUniquePtr<FFontRenderData> renderData;
};
