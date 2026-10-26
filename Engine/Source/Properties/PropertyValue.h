#pragma once

#include "Core/Math/Rotator.h"
#include "Core/Math/Vector3.h"

#include <variant>

// 두 값 중 하나를 보관한다. 기본 생성 시 FVector3의 각 성분은 0이다.
using FPropertyValue = std::variant<FVector3, FRotator>;
