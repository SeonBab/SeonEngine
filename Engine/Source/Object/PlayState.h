#pragma once

#include "Core/CoreTypes.h"

/** 플레이 참여 상태다. 전환 시점은 각 사용 클래스가 정한다. */
enum class EPlayState : uint8
{
	NotBegun,
	Beginning,
	Begun,
	Ended
};
