#pragma once

////////////////////////////////////////////////////////////////////////////////
// 플랫폼과 관계없이 애플리케이션의 메시지를 처리하는 공통 기반 클래스다.
// 메시지 처리 주체의 연결을 유지하도록 복사와 이동을 금지한다.
////////////////////////////////////////////////////////////////////////////////
class FGenericApplication
{
public:
	virtual ~FGenericApplication() = default;

	FGenericApplication(const FGenericApplication&) = delete;
	FGenericApplication& operator=(const FGenericApplication&) = delete;
	FGenericApplication(FGenericApplication&&) = delete;
	FGenericApplication& operator=(FGenericApplication&&) = delete;

	/** 이 스레드의 메시지를 처리한다. TimeDelta는 호출자가 전달하는 프레임 시간 간격(초)이다. */
	virtual void PumpMessages(float TimeDelta) = 0;

protected:
	FGenericApplication() = default;
};
