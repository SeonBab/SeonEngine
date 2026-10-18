#include "UserWidget.h"

#include <utility>

bool UUserWidget::CollectDrawElements(FUIDrawElementList& elements)
{
	if (!rootWidget) { return true; }

	return rootWidget->CollectDrawElements(elements);
}

bool UUserWidget::SetRootWidget(TUniquePtr<UWidget>& widget)
{
	if (!widget || widget.get() == this) { return false; }

	rootWidget = std::move(widget);

	return true;
}
