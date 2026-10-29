#pragma once

#include "Core/Templates/UniquePtr.h"
#include "Widget.h"

/** 게임 개발자가 UI 구성과 동작을 정의하는 기반이다. */
class UUserWidget : public UWidget
{
public:
	/**
	 * 게임 프레임의 표시 값을 갱신한다. deltaSeconds는 직전 프레임부터의 경과 초이며 기본 구현은 아무 작업도 하지 않는다.
	 * Subsystem이 요소 수집 전에 등록 순서로 호출한다. 루트나 자식의 Tick을 자동으로 재귀 호출하지 않는다.
	 * 호출 중 Subsystem의 등록·제거와 중첩 Tick/Draw는 금지한다. 표시 값 변경은 수집 전에 끝낸다.
	 */
	virtual void Tick(float deltaSeconds);

	/** 내부 루트에 같은 목록을 전달하고 결과를 반환한다. 루트가 없으면 true다. */
	[[nodiscard]] bool CollectDrawElements(FUIDrawElementList& elements) override;

	/**
	 * 새 루트의 소유권을 받고 기존 루트를 즉시 파괴한다. 성공하면 입력 포인터를 비운다.
	 * 빈 입력이나 자기 자신은 false이며 입력과 기존 루트를 유지한다. 교체 후 이전 루트의 포인터는 사용할 수 없다.
	 */
	[[nodiscard]] bool SetRootWidget(TUniquePtr<UWidget>& widget);

	/** 현재 루트를 빌려 수정한다. 없으면 nullptr이며 소유권은 유지한다. 교체/파괴 뒤에는 반환 포인터를 사용할 수 없다. */
	UWidget* GetRootWidget() { return rootWidget.get(); }

	/** 현재 루트를 빌려 읽는다. 없으면 nullptr이며, 교체, 파괴 시 반환 포인터는 무효다. */
	const UWidget* GetRootWidget() const { return rootWidget.get(); }

private:
	TUniquePtr<UWidget> rootWidget;
};
