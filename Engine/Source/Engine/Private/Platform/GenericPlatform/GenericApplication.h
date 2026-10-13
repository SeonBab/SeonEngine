#pragma once

#include "Templates/SharedPointer.h"

class FGenericWindow;
class FGenericApplicationMessageHandler;

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

	/** 메시지 수신자를 공유 소유로 교체한다. 빈 포인터는 Fatal이다. */
	virtual void SetMessageHandler(const TSharedPtr<FGenericApplicationMessageHandler>& inMessageHandler);

	/** OS 창을 만들지 않고 창 객체를 생성해 공유 소유 포인터로 반환한다. */
	virtual TSharedPtr<FGenericWindow> MakeWindow() = 0;

	/** 창의 OS 초기화를 수행한다. 호출자는 사용 전에 OS 창 핸들의 유효성을 확인한다. */
	virtual void InitializeWindow(const TSharedPtr<FGenericWindow>& window) = 0;

	/** 이 스레드의 메시지를 처리한다. TimeDelta는 호출자가 전달하는 프레임 시간 간격(초)이다. */
	virtual void PumpMessages(float TimeDelta) = 0;

protected:
	FGenericApplication();

	/** 현재 메시지 수신자를 공유 소유한다. 생성 시 기본 수신자를 연결한다. */
	TSharedPtr<FGenericApplicationMessageHandler> messageHandler;
};
