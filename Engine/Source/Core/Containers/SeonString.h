#pragma once

#include <string>
#include <string_view>

// UTF-8 문자열을 소유한다. 별칭 자체가 인코딩을 검사하지는 않는다.
using FString = std::string;

// 원본 문자열을 빌려 본다. 원본의 수명을 연장하지 않는다.
using FStringView = std::string_view;
