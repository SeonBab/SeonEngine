#pragma once

#include "Core/Math/Vector2.h"

class UTexture;

/** 글자 하나의 사각형과 배치에 필요한 API 독립적인 값이다. 기본 상태만으로 준비된 글리프를 뜻하지 않는다. */
struct FFontGlyph
{
	// 폰트 배율 1 기준 사각형의 픽셀 크기다. X는 오른쪽, Y는 아래쪽으로 증가한다.
	FVector2 size{};
	// 현재 글자 배치 위치(펜)에서 사각형 왼쪽 위까지의 픽셀 이동이다. 음수 오프셋도 허용한다.
	FVector2 offset{};
	// 배율 1에서 다음 펜까지의 전체 수평 픽셀 이동이다. 사각형 폭이나 SpriteFont의 XAdvance만을 뜻하지 않는다.
	float advanceX = 0.0f;

	// 아틀라스에서 글자 영역의 왼쪽 위/오른쪽 아래다. 각 성분은 0~1의 정규화된 UV다.
	FVector2 uvMin{};
	FVector2 uvMax{};

	// 비소유 참조다. nullptr는 미지정이며 조회부터 동기식 제출 완료까지 폰트 데이터와 아틀라스를 유지·교체 금지한다.
	const UTexture* atlas = nullptr;
};
