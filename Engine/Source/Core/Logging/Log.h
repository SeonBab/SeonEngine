#pragma once

#include "Core/Containers/SeonString.h"

#include <format>
#include <utility>

enum class ELogLevel : unsigned char
{
	Info,
	Warning,
	Error
};

namespace SE::Private
{
	// 완성된 메시지를 즉시 출력한다. 빈 카테고리는 General이며 문자열을 보관하지 않는다.
	void WriteLogMessage(FStringView category, ELogLevel level, FStringView message);
}

// 형식과 인자 타입을 컴파일할 때 검사한다.
// NOTE: 완성된 문자열은 형식 인수로 직접 넘기지 않고 "{}", message로 전달한다.
template<typename... ArgTypes>
void WriteLog(FStringView category, ELogLevel level, std::format_string<ArgTypes...> format, ArgTypes&&... args)
{
	SE::Private::WriteLogMessage(category, level, std::format(format, std::forward<ArgTypes>(args)...));
}
