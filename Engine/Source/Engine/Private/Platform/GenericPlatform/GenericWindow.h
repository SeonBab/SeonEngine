#pragma once

#include "CoreTypes.h"

////////////////////////////////////////////////////////////////////////////////
// 플랫폼에 관계없이 창을 사용하는 공통 기반 클래스다.
// OS 창과 객체의 연결이 유지되어야 하므로 복사와 이동을 금지한다.
////////////////////////////////////////////////////////////////////////////////
class FGenericWindow
{
public:
	virtual ~FGenericWindow() = default;

	FGenericWindow(const FGenericWindow&) = delete;
	FGenericWindow& operator=(const FGenericWindow&) = delete;
	FGenericWindow(FGenericWindow&&) = delete;
	FGenericWindow& operator=(FGenericWindow&&) = delete;

	/** 창을 만들어 화면에 보인다. 실패하면 확보한 자원을 정리하고 false를 반환한다. */
	[[nodiscard]] virtual bool Initialize() = 0;

	/** 창을 없애고 창에 쓴 자원을 정리한다. */
	virtual void Shutdown() = 0;

	/** 이 스레드의 메시지 큐를 처리한다. 여러 창을 지원할 때 애플리케이션으로 옮긴다. */
	virtual void PumpMessages() = 0;

	/** OS 창 핸들을 반환한다. 소유권을 넘기지 않으며, 창 파괴 후에는 nullptr이다. */
	virtual void* GetOSWindowHandle() const = 0;

	/** 마지막으로 처리한 메시지 기준 클라이언트 영역 너비다. */
	uint32 GetClientWidth() const { return clientWidth; }

	/** 마지막으로 처리한 메시지 기준 클라이언트 영역 높이다. */
	uint32 GetClientHeight() const { return clientHeight; }

	/** 마지막으로 처리한 메시지 기준 최소화 상태다. */
	bool IsMinimized() const { return bMinimized; }

	/** 닫기 요청 여부다. 실제 파괴는 엔진 종료 순서에서 Shutdown이 한다. */
	bool IsCloseRequested() const { return bCloseRequested; }

protected:
	FGenericWindow() = default;

	uint32 clientWidth = 0;
	uint32 clientHeight = 0;
	bool bMinimized = false;
	bool bCloseRequested = false;
};
