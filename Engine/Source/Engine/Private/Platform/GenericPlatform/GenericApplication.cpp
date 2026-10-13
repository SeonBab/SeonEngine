#include "Platform/GenericPlatform/GenericApplication.h"

#include "Logging/LogMacros.h"
#include "Platform/GenericPlatform/GenericApplicationMessageHandler.h"

namespace
{
	SE_DECLARE_LOG_CATEGORY(LogGenericApplication);
}

FGenericApplication::FGenericApplication()
	: messageHandler(MakeShared<FGenericApplicationMessageHandler>())
{
}

void FGenericApplication::SetMessageHandler(const TSharedPtr<FGenericApplicationMessageHandler>& inMessageHandler)
{
	if (!inMessageHandler)
	{
		SE_LOG(LogGenericApplication, Fatal, "FGenericApplication::SetMessageHandler requires a message handler");
	}

	messageHandler = inMessageHandler;
}
