#pragma once

#include "CoreTypes.h"

/** 부모 변경 시 유지할 변환 기준이다. */
enum class ETransformRule : uint8
{
	KeepWorld,
	KeepLocal
};
