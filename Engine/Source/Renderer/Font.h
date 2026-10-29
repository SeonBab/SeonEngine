#pragma once

#include "Core/CoreTypes.h"
#include "Core/Templates/UniquePtr.h"
#include "Object.h"

class FFontRenderData;
struct FFontGlyph;

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

	/**
	 * 배율 1에서 다음 줄까지의 수직 픽셀 이동을 조회한다. 성공 값은 유한한 양수다.
	 * 미설정/미준비/잘못된 값은 로그/false이며 출력 인자는 유지한다. 같은 스레드에서 데이터 교체 없이 호출한다.
	 */
	[[nodiscard]] bool GetLineSpacing(float& outLineSpacing) const;

	/**
	 * Unicode 스칼라 값 하나의 배율 1 메트릭을 조회한다. 없는 글자는 폰트의 대체 문자를 사용한다.
	 * 실패는 로그/false이며 outGlyph를 유지한다. 성공 결과의 아틀라스는 비소유 참조이므로
	 * 동기식 제출 완료까지 이 폰트의 렌더 데이터를 유지하고 교체하지 않는다. 문자열 배치는 수행하지 않는다.
	 */
	[[nodiscard]] bool TryGetGlyph(uint32 codePoint, FFontGlyph& outGlyph) const;

private:
	TUniquePtr<FFontRenderData> renderData;
};
