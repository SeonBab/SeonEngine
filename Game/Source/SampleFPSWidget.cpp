#include "SampleFPSWidget.h"

#include "Core/Logging/Log.h"
#include "UI/TextWidget.h"

#include <cmath>
#include <format>
#include <utility>

bool USampleFPSWidget::Initialize(TSharedPtr<UFont> font)
{
	TUniquePtr<UTextWidget> textWidget = MakeUnique<UTextWidget>();
	if (!textWidget->SetFont(std::move(font))) { return false; }
	textWidget->SetText("FPS: --");
	textWidget->SetPosition({20.0f, 168.0f});

	TUniquePtr<UWidget> root = std::move(textWidget);
	if (!SetRootWidget(root)) { return false; }

	elapsedSeconds = 0.0;
	frameCount = 0;

	return true;
}

void USampleFPSWidget::Tick(float deltaSeconds)
{
	if (!std::isfinite(deltaSeconds))
	{
		WriteLog("SampleGame", ELogLevel::Error, "FPS updates require finite elapsed seconds.");
		return;
	}
	if (deltaSeconds < 0.0f)
	{
		WriteLog("SampleGame", ELogLevel::Error, "FPS updates require non-negative elapsed seconds.");
		return;
	}
	if (deltaSeconds == 0.0f) { return; }

	UTextWidget* textWidget = dynamic_cast<UTextWidget*>(GetRootWidget());
	if (!textWidget) { return; }

	elapsedSeconds += static_cast<double>(deltaSeconds);
	++frameCount;
	constexpr double UpdateIntervalSeconds = 0.5;
	if (elapsedSeconds < UpdateIntervalSeconds) { return; }

	const double fps = static_cast<double>(frameCount) / elapsedSeconds;
	textWidget->SetText(std::format("FPS: {:.1f}", fps));
	elapsedSeconds = 0.0;
	frameCount = 0;
}
