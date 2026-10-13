#pragma once

#if !SE_PLATFORM_WINDOWS
#error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
#endif

#include "Platform/GenericPlatform/GenericWindow.h"

class FWindowsApplication;

namespace SE::Private
{
	struct FWindowsWindowProc;
}

////////////////////////////////////////////////////////////////////////////////
// Windows의 OS 창 하나를 관리한다. Win32 타입과 호출은 구현 파일에 둔다.
////////////////////////////////////////////////////////////////////////////////
class FWindowsWindow final : public FGenericWindow
{
	friend struct SE::Private::FWindowsWindowProc;

public:
	/** 연결할 Application은 이 창 객체보다 오래 살아 있어야 한다. */
	explicit FWindowsWindow(FWindowsApplication& inApplication);
	~FWindowsWindow() override = default;

	[[nodiscard]] bool Initialize() override;
	void Shutdown() override;
	void* GetOSWindowHandle() const override { return nativeHandle; }

	/** 메시지로 받은 크기와 최소화 상태를 기록한다. */
	void UpdateClientSize(uint32 width, uint32 height, bool bInMinimized);

	/** 닫기 요청만 기록한다. 실제 파괴는 Shutdown에서 수행한다. */
	void RequestClose();

private:
	// 생성 시 연결한 Application을 빌린다. 소유하거나 다시 연결하지 않는다.
	FWindowsApplication& owningApplication;

	void* nativeHandle = nullptr;

	// 창이 파괴되어도 클래스 등록은 남을 수 있어 핸들과 별도로 기록한다.
	bool bClassRegistered = false;
};
