#pragma once

#include <chrono>

class FrameTimer
{
public:
	/** 생성 시 첫 측정의 기준 시점을 저장한다. */
	FrameTimer();
	virtual ~FrameTimer() = default;

	FrameTimer(const FrameTimer&) = delete;
	FrameTimer& operator=(const FrameTimer&) = delete;
	FrameTimer(FrameTimer&&) = delete;
	FrameTimer& operator=(FrameTimer&&) = delete;

	/** 직전 측정(첫 호출은 생성)부터 경과한 초를 반환하고 기준 시점을 갱신한다. */
	float Tick();

private:
	std::chrono::steady_clock::time_point previousTime;
};
