#include "FrameTimer.h"

FFrameTimer::FFrameTimer()
	: previousTime(std::chrono::steady_clock::now())
{
}

float FFrameTimer::Tick()
{
	const std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	const float deltaSeconds = std::chrono::duration<float>(currentTime - previousTime).count();
	previousTime = currentTime;

	return deltaSeconds;
}
