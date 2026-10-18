#pragma once

#include <memory>
#include <utility>

template<typename T>
using TUniquePtr = std::unique_ptr<T>;

/** 생성 인수를 전달하여 객체를 만들고 단독 소유 포인터를 반환한다. */
template<typename T, typename... ArgsType>
TUniquePtr<T> MakeUnique(ArgsType&&... args)
{
	return std::make_unique<T>(std::forward<ArgsType>(args)...);
}
