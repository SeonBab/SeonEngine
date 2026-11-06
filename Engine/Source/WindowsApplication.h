#pragma once

#include "Input/InputState.h"

#include <Windows.h>

/** 메인 스레드에서 생성·초기화·사용·소멸하며, 자신이 만든 창과 창 클래스를 소유한다. */
class WindowsApplication
{
	// 생성과 소멸
public:
	WindowsApplication() = default;
	~WindowsApplication();

	WindowsApplication(const WindowsApplication&) = delete;
	WindowsApplication& operator=(const WindowsApplication&) = delete;
	WindowsApplication(WindowsApplication&&) = delete;
	WindowsApplication& operator=(WindowsApplication&&) = delete;

	/** 창 클래스를 등록하고 창을 만든다. 이미 등록한 객체의 재호출은 실패하며 상태를 유지한다. */
	bool Initialize();

private:
	bool Create();

	// 창과 메시지
public:
	/**
	 * 대기 중인 메시지를 처리한다. 프레임당 한 번 호출하며 시작할 때 이전 프레임의 키 이벤트를 비운다.
	 * 종료 요청 시 outExitCode에 종료 코드를 저장하고 false를 반환한다.
	 */
	bool ProcessMessages(int& outExitCode);

	/**
	 * 소유한 창 핸들을 빌려주며 창이 없으면 nullptr이다.
	 * 초기화 실패 후 창 정리도 실패하면 핸들을 유지하고 소멸자에서 다시 정리한다.
	 * 창 파괴 또는 이 객체 소멸 후에는 빌린 핸들을 사용하지 않는다.
	 */
	HWND GetWindowHandle() const;

	/** 클라이언트 영역 크기를 조회한다. 실패하면 출력값을 유지한다. */
	bool GetClientSize(int& outWidth, int& outHeight) const;

private:
	static LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND window = nullptr;
	HINSTANCE instance = nullptr;

	// 창 생성에 실패해도, 자신이 등록한 창 클래스는 소멸 시 해제한다.
	bool isWindowClassRegistered = false;

	// 입력
public:
	/**
	 * 창이 수집한 키 상태를 읽기 전용으로 빌려준다. 이 객체가 살아 있는 동안 유효하다.
	 * 상태는 ProcessMessages 호출 중에 갱신된다.
	 */
	const InputState& GetInputState() const;

private:
	InputState inputState;
};

