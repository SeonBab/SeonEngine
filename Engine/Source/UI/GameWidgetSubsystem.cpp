#include "GameWidgetSubsystem.h"

#include "Renderer/Renderer.h"
#include "UI/UIDrawElementList.h"

#include <utility>

#pragma region Widgets

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

#pragma endregion

#pragma region Updating

void FGameWidgetSubsystem::Tick(float deltaSeconds)
{
	for (const TUniquePtr<UUserWidget>& widget : widgets)
	{
		widget->Tick(deltaSeconds);
	}
}

#pragma endregion

#pragma region Drawing

bool FGameWidgetSubsystem::Draw(IRenderer& renderer)
{
	FUIDrawElementList elements;

	for (const TUniquePtr<UUserWidget>& widget : widgets)
	{
		if (!widget->CollectDrawElements(elements)) { return false; }
	}

	return renderer.DrawUI(elements);
}

#pragma endregion
