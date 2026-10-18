#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Widget.h"

/** 게임 개발자가 UI 구성과 동작을 정의하는 기반이다. */
class UUserWidget : public UWidget
{
public:
	/** 내부 루트에 그리기를 요청하고 결과를 전달한다. 루트가 없으면 true이며 Renderer는 보관하지 않는다. */
	[[nodiscard]] bool Draw(FRenderer& renderer) override;

	/**
	 * 새 루트의 소유권을 받고 기존 루트를 즉시 파괴한다. 성공하면 입력 포인터를 비운다.
	 * 빈 입력이나 자기 자신은 false이며 입력과 기존 루트를 유지한다. 교체 후 이전 루트의 포인터는 사용할 수 없다.
	 */
	[[nodiscard]] bool SetRootWidget(TUniquePtr<UWidget>& widget);

	/** 현재 루트를 빌려 읽는다. 없으면 nullptr이며, 교체·파괴 시 반환 포인터는 무효다. */
	const UWidget* GetRootWidget() const { return rootWidget.get(); }

private:
	TUniquePtr<UWidget> rootWidget;
};
