#pragma once

#if !SE_PLATFORM_WINDOWS
#error "Windows 헤더가 Windows가 아닌 빌드에서 include됐습니다. SE_PLATFORM_HEADER_NAME을 확인하세요."
#endif

#include "Platform/GenericPlatform/GenericWindow.h"

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
	FWindowsWindow() = default;
	~FWindowsWindow() override = default;

	[[nodiscard]] bool Initialize() override;
	void Shutdown() override;
	void PumpMessages() override;
	void* GetOSWindowHandle() const override { return nativeHandle; }

private:
	void* nativeHandle = nullptr;

	// 창이 파괴되어도 클래스 등록은 남을 수 있어 핸들과 별도로 기록한다.
	bool bClassRegistered = false;
};
