#pragma once

#include <unordered_map>

// 엔진의 해시 맵이다. std::unordered_map의 별칭이라 멤버 함수는 STL 이름(find, emplace 등)을 그대로 쓴다.
// 메모리 추적이 필요해지면 이 별칭에 엔진 할당자를 넣는다.
template<typename KeyType, typename ValueType>
using THashMap = std::unordered_map<KeyType, ValueType>;
