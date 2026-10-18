#include "Log.h"

#include <cstdio>

void SE::Private::WriteLogMessage(FStringView category, ELogLevel level, FStringView message)
{
	const char* prefix = "[Unknown] ";
	switch (level)
	{
		case ELogLevel::Info:
			prefix = "[Info] ";
			break;

		case ELogLevel::Warning:
			prefix = "[Warning] ";
			break;

		case ELogLevel::Error:
			prefix = "[Error] ";
			break;
	}

	if (category.empty())
	{
		category = "General";
	}
	std::fputc('[', stderr);
	std::fwrite(category.data(), 1, category.size(), stderr);
	std::fputc(']', stderr);
	std::fputs(prefix, stderr);
	if (!message.empty())
	{
		std::fwrite(message.data(), 1, message.size(), stderr);
	}
	std::fputc('\n', stderr);
}
