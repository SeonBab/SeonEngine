#pragma once

/** 렌더러별 폰트 자원의 공통 기반이다. 실제 자원은 파생 타입이 소유한다. */
class FFontRenderData
{
public:
	/** 기반 포인터로 파괴해도 파생 타입의 자원을 정리한다. */
	virtual ~FFontRenderData() = default;
};
