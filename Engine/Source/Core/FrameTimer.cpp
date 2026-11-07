#include "FrameTimer.h"

FrameTimer::FrameTimer()
	: previousTime(std::chrono::steady_clock::now())
{

}

float FrameTimer::Tick()
{
	const std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	const float deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();

	previousTime = currentTime;

	return deltaTime;
}
