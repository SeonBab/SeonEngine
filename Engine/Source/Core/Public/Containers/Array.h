#pragma once

#include <vector>

// 엔진의 동적 배열이다. std::vector의 별칭이라 멤버 함수는 STL 이름(push_back, size 등)을 그대로 쓴다.
// 메모리 추적이 필요해지면 이 별칭에 엔진 할당자를 넣는다.
template<typename T>
using TArray = std::vector<T>;
