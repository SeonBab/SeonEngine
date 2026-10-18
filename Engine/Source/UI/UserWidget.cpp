#include "UserWidget.h"

#include <utility>

bool UUserWidget::Draw(FRenderer& renderer)
{
	if (!rootWidget) { return true; }

	return rootWidget->Draw(renderer);
}

bool UUserWidget::SetRootWidget(TUniquePtr<UWidget>& widget)
{
	if (!widget || widget.get() == this) { return false; }

	rootWidget = std::move(widget);

	return true;
}
