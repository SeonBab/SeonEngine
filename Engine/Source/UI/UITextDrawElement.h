#pragma once

#include "Core/Containers/SeonString.h"
#include "Core/Math/Vector2.h"

class UFont;

/**
 * 현재 출력 영역에 표시할 텍스트 한 건의 그리기 정보다. 문자열은 소유하며 폰트는 빌린다.
 * 수집부터 제출 완료까지 폰트 자산과 렌더 데이터를 유지한다. 값과 GPU 자원의 유효성은 검사하지 않는다.
 */
struct FUITextDrawElement
{
	// nullptr는 폰트 미지정 상태다. 자산의 수명을 연장하지 않는다.
	const UFont* font = nullptr;
	// 중간 NUL이 없는 올바른 UTF-8 문자열 사본이다. 기본값은 빈 문자열이며 배처는 탭을 거부한다.
	// 공백은 위치 이동만, LF는 다음 줄, CR은 무시한다. 자동 줄바꿈이나 복잡한 문자 shaping은 하지 않는다.
	FString text;
	// 현재 출력 영역 왼쪽 위를 원점으로 하는 첫 줄 펜의 픽셀 위치다. 글리프 offset을 더해 사각형을 배치한다.
	// 음수 offset을 보정하지 않으므로 실제 글자 픽셀은 이 위치의 왼쪽/위에도 나타날 수 있다.
	FVector2 position{};
	// 각 성분은 0~1의 RGBA다. RGB에 알파를 미리 곱하지 않으며 기본값은 불투명 흰색이다.
	float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	// 준비된 폰트 크기에 적용하는 양수의 유한 배율이다. 1은 원래 크기다.
	float scale = 1.0f;
};
