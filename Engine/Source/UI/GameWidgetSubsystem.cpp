#include "GameWidgetSubsystem.h"

#include <utility>

bool FGameWidgetSubsystem::Draw(FRenderer& renderer)
{
	for (const TUniquePtr<UUserWidget>& widget : widgets)
	{
		if (!widget->Draw(renderer)) { return false; }
	}

	return true;
}

bool FGameWidgetSubsystem::AddWidget(TUniquePtr<UUserWidget>& widget)
{
	if (!widget) { return false; }

	widgets.push_back(std::move(widget));

	return true;
}

bool FGameWidgetSubsystem::RemoveWidget(UUserWidget* widget)
{
	if (widget == nullptr) { return false; }

	for (TArray<TUniquePtr<UUserWidget>>::iterator it = widgets.begin(); it != widgets.end(); ++it)
	{
		if (it->get() == widget)
		{
			widgets.erase(it);

			return true;
		}
	}

	return false;
}
