#pragma once

#include <memory>
#include <utility>

// 객체를 혼자 소유하는 포인터다. std::unique_ptr의 별칭이라 멤버 함수는 STL 이름(get, reset 등)을 그대로 쓴다.
// 소유하지 않는 쪽은 일반 포인터나 레퍼런스로 빌려 쓴다.
template<typename T>
using TUniquePtr = std::unique_ptr<T>;

/**
 * T를 만들어 TUniquePtr로 돌려준다.
 * 함수에는 별칭을 붙일 수 없어서 std::make_unique를 부르는 함수로 둔다.
 * 인자를 그대로 넘기므로 배열(MakeUnique<int32[]>(count))도 표준과 같게 동작한다.
 */
template<typename T, typename... ArgTypes>
[[nodiscard]] TUniquePtr<T> MakeUnique(ArgTypes&&... args)
{
	return std::make_unique<T>(std::forward<ArgTypes>(args)...);
}
