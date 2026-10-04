#pragma once

// 플랫폼 창의 공통 헤더다. 선언은 여기에 두고, 구현은 플랫폼 폴더의 .cpp(Windows/WindowsWindow.cpp)에 둔다.
// 플랫폼 독립 코드가 include하므로 Win32 타입(HWND 등)을 쓰지 않는다.

namespace SE::Private
{
	// 플랫폼의 메시지 처리 함수를 담는 보조 구조체. 정의는 플랫폼 구현 .cpp에 있다
	struct FWindowsWindowProc;
}

////////////////////////////////////////////////////////////////////////////////
// OS 창 하나를 관리한다.
// 창의 메시지를 처리하는 OS 쪽 코드가 이 객체의 주소를 들고 있으므로, 복사와 이동을 막는다.
////////////////////////////////////////////////////////////////////////////////
class FWindow
{
	// 메시지 처리 보조 구조체가 창 생성 / 파괴 중에 nativeHandle을, 닫기 요청 때 bCloseRequested를 바꾼다
	friend struct SE::Private::FWindowsWindowProc;

public:
	FWindow()  = default;
	~FWindow() = default;

	FWindow(const FWindow&)            = delete;
	FWindow& operator=(const FWindow&) = delete;
	FWindow(FWindow&&)                 = delete;
	FWindow& operator=(FWindow&&)      = delete;

	/**
	 * 창을 만들어 화면에 보인다.
	 *
	 * @return 실패하면 false. 이때 이미 확보한 자원은 정리된 상태다.
	 */
	[[nodiscard]] bool Initialize();

	/** 창을 없애고 창에 쓴 자원을 정리한다. */
	void Shutdown();

	/** 이 스레드의 메시지 큐에 쌓인 메시지를 모두 꺼내 창 프로시저로 보낸다. */
	void PumpMessages();

	/** 사용자가 창을 닫으려 했는가. 창은 아직 남아 있으며, 실제로 없애는 것은 Shutdown이다. */
	bool IsCloseRequested() const { return bCloseRequested; }

private:
	// OS 창 핸들. Windows에서는 HWND이며, 구현 .cpp에서만 실제 타입으로 바꿔 쓴다.
	void* nativeHandle = nullptr;

	// 닫기 버튼 등으로 닫기 요청(WM_CLOSE)이 왔는가. 메시지 처리 보조 구조체가 세운다
	bool bCloseRequested = false;
};
