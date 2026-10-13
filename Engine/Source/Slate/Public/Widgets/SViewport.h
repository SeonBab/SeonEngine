#pragma once

#include "Rendering/RenderingCommon.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SWidget.h"

////////////////////////////////////////////////////////////////////////////////
// 게임 화면 위젯의 최소 기반이다. 화면 인터페이스를 비소유 참조로 보관한다.
////////////////////////////////////////////////////////////////////////////////
class SViewport : public SWidget
{
public:
	/** 화면 인터페이스를 약하게 보관한다. 빈 포인터는 연결을 해제한다. */
	void SetViewportInterface(const TSharedPtr<ISlateViewport>& inViewportInterface)
	{
		viewportInterface = inViewportInterface;
	}

	/** 키 누름을 화면에 전달한다. 화면이 없거나 만료되면 미처리다. 호출 동안 화면 수명을 유지한다. */
	FReply OnKeyDown(const FKeyEvent& inKeyEvent) override
	{
		TSharedPtr<ISlateViewport> viewport = viewportInterface.lock();
		if (!viewport)
		{
			return FReply::Unhandled();
		}

		return viewport->OnKeyDown(inKeyEvent);
	}

	/** 키 뗌을 화면에 전달한다. 화면이 없거나 만료되면 미처리다. 호출 동안 화면 수명을 유지한다. */
	FReply OnKeyUp(const FKeyEvent& inKeyEvent) override
	{
		TSharedPtr<ISlateViewport> viewport = viewportInterface.lock();
		if (!viewport)
		{
			return FReply::Unhandled();
		}

		return viewport->OnKeyUp(inKeyEvent);
	}

private:
	TWeakPtr<ISlateViewport> viewportInterface;
};
