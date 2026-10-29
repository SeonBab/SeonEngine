#pragma once

#include "Core/CoreTypes.h"
#include "Core/Templates/UniquePtr.h"

class UTexture2D;
struct FFontGlyph;

/** 폰트 아틀라스를 소유하고 공통 조회를 제공한다. 백엔드 고유 폰트 자원은 파생 타입이 소유한다. */
class FFontRenderData
{
public:
	/** 기반 포인터로 파괴해도 파생 타입의 자원을 정리한다. */
	virtual ~FFontRenderData();

	/**
	 * 배율 1에서 다음 줄까지의 수직 이동을 픽셀 단위로 조회한다. 성공 값은 유한한 양수다.
	 * 미준비/잘못된 값은 로그/false이며 outLineSpacing을 유지한다. 폰트 자원이나 GPU 상태는 변경하지 않는다.
	 */
	[[nodiscard]] virtual bool GetLineSpacing(float& outLineSpacing) const = 0;

	/**
	 * Unicode 스칼라 값 하나를 조회하며 없는 글자는 설정된 대체 문자를 사용한다.
	 * size/offset은 배율 1 픽셀, UV는 아틀라스 정규화 좌표, advanceX는 다음 펜까지의 전체 이동이다.
	 * 음수 offset을 보존하며 문자열 펜 이동/보정은 하지 않는다. 성공한 atlas는 이 데이터가 소유하는 비소유 참조다.
	 * 잘못된 입력/미준비/대체 문자도 없음/유효하지 않은 메트릭은 로그/false이며 outGlyph를 유지한다.
	 * 조회부터 동기식 제출 완료까지 같은 스레드에서 이 데이터와 아틀라스를 유지하고 교체하지 않는다.
	 */
	[[nodiscard]] virtual bool TryGetGlyph(uint32 codePoint, FFontGlyph& outGlyph) const = 0;

protected:
	/** 아틀라스가 없는 빈 데이터로 생성한다. 자원 생성은 백엔드가 후보 데이터에서 완료한다. */
	FFontRenderData();

	// 폰트의 고정 아틀라스 자산을 단독 소유한다. 외부에는 글리프 조회 결과로만 빌려 제공한다.
	// nullptr는 미준비다. 폰트 데이터 교체/해제 전에 모든 비소유 사용을 끝내야 한다.
	TUniquePtr<UTexture2D> atlas;
};
