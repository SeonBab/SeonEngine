#pragma once

#include "Core/Containers/Array.h"
#include "Core/Templates/UniquePtr.h"
#include "UserWidget.h"

class FRenderer;

/** 화면에 등록된 사용자 위젯을 소유한다. */
class FGameWidgetSubsystem
{
public:
	/**
	 * 등록 순서대로 요소를 수집한 뒤 Renderer에 제출한다. 수집 실패 시 제출하지 않으며 그리기 실패도 false로 전달한다. 빈 목록은 true다.
	 * 수집 시작부터 그리기 완료까지 위젯 등록·제거, 루트·텍스처 교체는 허용하지 않는다. 요소 목록은 호출 안에서만 유지한다.
	 */
	[[nodiscard]] bool Draw(FRenderer& renderer);

	/** 성공하면 위젯의 소유권을 받고 입력 포인터를 비운다. 빈 입력이면 목록을 유지하고 false를 반환한다. */
	[[nodiscard]] bool AddWidget(TUniquePtr<UUserWidget>& widget);

	/** 등록된 위젯을 제거하고 즉시 파괴한다. 없으면 false를 반환한다. 성공 후 전달한 포인터를 사용하면 안 된다. */
	[[nodiscard]] bool RemoveWidget(UUserWidget* widget);

private:
	TArray<TUniquePtr<UUserWidget>> widgets;
};
