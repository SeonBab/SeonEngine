#pragma once

#include "Core/CoreTypes.h"
#include "Core/Templates/SharedPtr.h"
#include "UI/UserWidget.h"

class UFont;

/** 게임 프레임 시간을 구간별로 누적해 내부 텍스트에 평균 FPS를 표시하는 샘플 위젯이다. */
class USampleFPSWidget : public UUserWidget
{
public:
	/**
	 * 공유 폰트로 TextWidget 루트를 준비하고 FPS: --를 (20,168)에 표시한다. 폰트의 GPU 준비 여부는 검사하지 않는다.
	 * 성공하면 기존 루트와 집계 구간을 교체한다. 빈 폰트는 로그/false이며 기존 루트와 집계 값은 유지한다.
	 * 폰트는 루트가 공유 소유한다. 호출자의 공유 참조는 유지하며 수집·제출 중에는 초기화하지 않는다.
	 */
	[[nodiscard]] bool Initialize(TSharedPtr<UFont> font);

	/**
	 * 양수·유한한 경과 초와 프레임 수를 누적하고 0.5초 이상이면 실제 누적 구간의 평균 FPS를 소수 한 자리로 표시한다.
	 * 0초는 무시하고 음수·비유한 값은 로그 후 무시한다. 현재 루트가 TextWidget이 아니면 집계와 표시를 건너뛴다.
	 * 루트 주소는 호출마다 다시 조회한다. 값이 무시되면 기존 집계와 표시를 유지하며 할당 등의 예외는 전파한다.
	 */
	void Tick(float deltaSeconds) override;

private:
	// 마지막 표시 갱신 뒤 받은 유효한 프레임 시간의 합(초)과 프레임 수다. 고정 0.5초로 나누지 않는다.
	double elapsedSeconds = 0.0;
	uint64 frameCount = 0;
};
