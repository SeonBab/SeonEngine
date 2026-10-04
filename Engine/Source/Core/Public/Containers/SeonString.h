#pragma once

// 파일 이름을 String.h로 짓지 않는다.
// Windows는 파일 이름의 대소문자를 구분하지 않아서, 다른 헤더가 따옴표로 include한 "string.h"가 이 파일로 잡힐 수 있다.

#include <string>
#include <string_view>

// 엔진 문자열은 UTF-8로 저장한다. 이 별칭 자체는 인코딩을 검사하지 않는다.
// Windows API에 넘길 때만 UTF-16으로 변환한다.
using FString = std::string;

// 문자열을 복사하지 않고 빌려 보는 참조다. 원본 문자열보다 오래 들고 있으면 안 된다
using FStringView = std::string_view;
