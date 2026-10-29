#pragma once

#include "Core/Containers/Array.h"
#include "Core/Templates/UniquePtr.h"
#include "UserWidget.h"

class IRenderer;

/** 화면에 등록된 사용자 위젯을 소유한다. */
class FGameWidgetSubsystem
{
#pragma region Widgets

public:
	/** 성공하면 위젯의 소유권을 받고 입력 포인터를 비운다. 빈 입력은 false다. Tick과 수집·제출 중에는 호출하지 않는다. */
	[[nodiscard]] bool AddWidget(TUniquePtr<UUserWidget>& widget);

	/** 등록된 위젯을 제거하고 즉시 파괴한다. 없으면 false다. Tick과 수집·제출 중 호출 및 파괴 후 포인터 재사용은 금지한다. */
	[[nodiscard]] bool RemoveWidget(UUserWidget* widget);

private:
	TArray<TUniquePtr<UUserWidget>> widgets;

#pragma endregion

#pragma region Updating

public:
	/**
	 * 게임 프레임의 경과 초를 등록 순서대로 각 UserWidget에 전달한다. 빈 목록은 아무 작업도 하지 않는다.
	 * GameEngine이 월드 갱신 뒤 요소 수집 전에 한 번 호출하며 그리기는 하지 않는다.
	 * 순회 중 등록·제거와 중첩 Tick/Draw는 금지한다. 변경을 자동 감지하거나 보류하지 않으며 예외는 전파한다.
	 */
	void Tick(float deltaSeconds);

#pragma endregion

#pragma region Drawing

public:
	/**
	 * 등록 순서대로 요소를 수집한 뒤 Renderer에 제출한다. 수집 실패 시 제출하지 않으며 그리기 실패도 false로 전달한다. 빈 목록은 true다.
	 * 수집 시작부터 그리기 완료까지 위젯 등록, 제거, 루트, 텍스처 교체는 허용하지 않는다. 요소 목록은 호출 안에서만 유지한다.
	 */
	[[nodiscard]] bool Draw(IRenderer& renderer);

#pragma endregion
};
