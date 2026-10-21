#pragma once

#include <memory>
#include <utility>

template<typename T>
using TSharedPtr = std::shared_ptr<T>;

/** 생성 인수를 전달하여 객체를 만들고 공유 소유 포인터를 반환한다. */
template<typename T, typename... ArgsType>
TSharedPtr<T> MakeShared(ArgsType&&... args)
{
	return std::make_shared<T>(std::forward<ArgsType>(args)...);
}
