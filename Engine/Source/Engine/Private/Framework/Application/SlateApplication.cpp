#include "Framework/Application/SlateApplication.h"

#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Logging/LogMacros.h"
#include "Platform/GenericPlatform/GenericApplication.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogSlate);
}

TSharedPtr<FSlateApplication> FSlateApplication::currentApplication = nullptr;
TSharedPtr<FGenericApplication> FSlateApplication::platformApplication = nullptr;

TSharedPtr<FSlateApplication> FSlateApplication::Create(const TSharedPtr<FGenericApplication>& inPlatformApplication)
{
	if (!inPlatformApplication)
	{
		SE_LOG(LogSlate, Fatal, "FSlateApplication::Create requires a platform application");
	}

	if (currentApplication)
	{
		SE_LOG(LogSlate, Fatal, "FSlateApplication::Create requires no existing application");
	}

	platformApplication = inPlatformApplication;

	// private 생성자이므로 클래스 내부에서 생성하고 즉시 공유 소유 포인터에 넘긴다.
	currentApplication = TSharedPtr<FSlateApplication>(new FSlateApplication());
	platformApplication->SetMessageHandler(currentApplication);

	return currentApplication;
}

FSlateApplication& FSlateApplication::Get()
{
	if (!currentApplication)
	{
		SE_LOG(LogSlate, Fatal, "FSlateApplication::Get requires an existing application");
	}

	return *currentApplication;
}

void FSlateApplication::Shutdown()
{
	if (platformApplication)
	{
		platformApplication->SetMessageHandler(MakeShared<FGenericApplicationMessageHandler>());
	}

	currentApplication.reset();
	platformApplication.reset();
}

FSlateApplication::FSlateApplication() = default;

bool FSlateApplication::OnKeyDown(int32 keyCode, uint32 characterCode, bool bIsRepeat)
{
	const FKey key = FInputKeyManager::Get().GetKeyFromCodes(static_cast<uint32>(keyCode), characterCode);
	const FKeyEvent keyEvent(key, bIsRepeat);

	return ProcessKeyDownEvent(keyEvent);
}

bool FSlateApplication::OnKeyUp(int32 keyCode, uint32 characterCode, bool bIsRepeat)
{
	const FKey key = FInputKeyManager::Get().GetKeyFromCodes(static_cast<uint32>(keyCode), characterCode);
	const FKeyEvent keyEvent(key, bIsRepeat);

	return ProcessKeyUpEvent(keyEvent);
}

bool FSlateApplication::ProcessKeyDownEvent([[maybe_unused]] const FKeyEvent& inKeyEvent)
{
	return false;
}

bool FSlateApplication::ProcessKeyUpEvent([[maybe_unused]] const FKeyEvent& inKeyEvent)
{
	return false;
}
