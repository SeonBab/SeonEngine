#pragma once

#include <memory>
#include <utility>

// 여러 소유자가 함께 소유하는 포인터다. 마지막 소유자가 사라질 때 객체가 지워진다.
// 소유자를 하나로 정할 수 없을 때만 쓰고, 편의를 위해 쓰지 않는다.
// 참조 카운트 관리만 스레드 안전하며, 가리키는 객체의 데이터는 보호하지 않는다.
template<typename T>
using TSharedPtr = std::shared_ptr<T>;

// TSharedPtr가 가리키는 객체를 소유하지 않고 관찰한다.
// 사용할 때는 expired()로 먼저 확인하지 않고 lock()의 결과를 확인한다.
// 확인과 사용 사이에 다른 스레드가 마지막 소유권을 놓으면 객체가 사라질 수 있기 때문이다.
template<typename T>
using TWeakPtr = std::weak_ptr<T>;

/**
 * T를 만들어 TSharedPtr로 돌려준다.
 * 함수에는 별칭을 붙일 수 없어서 std::make_shared를 부르는 함수로 둔다.
 */
template<typename T, typename... ArgTypes>
[[nodiscard]] TSharedPtr<T> MakeShared(ArgTypes&&... args)
{
	return std::make_shared<T>(std::forward<ArgTypes>(args)...);
}
