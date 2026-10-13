#pragma once

#include "Platform/GenericPlatform/GenericApplicationMessageHandler.h"
#include "Templates/SharedPointer.h"

class FGenericApplication;
struct FKeyEvent;

////////////////////////////////////////////////////////////////////////////////
// 플랫폼 입력을 수신하고 공통 입력 이벤트 처리를 담당하는 계층이다.
// UI 실행과 ImGui 연결은 아직 구현하지 않았다.
////////////////////////////////////////////////////////////////////////////////
class FSlateApplication : public FGenericApplicationMessageHandler
{
public:
	~FSlateApplication() override = default;

	FSlateApplication(const FSlateApplication&) = delete;
	FSlateApplication& operator=(const FSlateApplication&) = delete;
	FSlateApplication(FSlateApplication&&) = delete;
	FSlateApplication& operator=(FSlateApplication&&) = delete;

	/** 플랫폼을 공유 소유하고 Slate를 생성해 메시지 수신자로 등록한다. 빈 플랫폼 / 중복 생성은 Fatal이다. */
	static TSharedPtr<FSlateApplication> Create(const TSharedPtr<FGenericApplication>& inPlatformApplication);

	/** 현재 Slate의 비소유 참조를 반환한다. 생성 전 / 종료 후 조회는 Fatal이다. */
	static FSlateApplication& Get();

	/** 플랫폼 수신자를 기본 핸들러로 교체하고 Slate / 플랫폼의 공유 소유권을 놓는다. 준비 전 / 반복 호출을 허용하며 OS 종료는 수행하지 않는다. */
	static void Shutdown();

	/** 플랫폼 키 누름을 공통 키 이벤트로 변환해 전달하고 처리 여부를 반환한다. */
	bool OnKeyDown(int32 keyCode, uint32 characterCode, bool bIsRepeat) override;

	/** 플랫폼 키 뗌을 공통 키 이벤트로 변환해 전달하고 처리 여부를 반환한다. */
	bool OnKeyUp(int32 keyCode, uint32 characterCode, bool bIsRepeat) override;

	/** 공통 키 누름 이벤트를 처리하고 처리 여부를 반환한다. */
	bool ProcessKeyDownEvent(const FKeyEvent& inKeyEvent);

	/** 공통 키 뗌 이벤트를 처리하고 처리 여부를 반환한다. */
	bool ProcessKeyUpEvent(const FKeyEvent& inKeyEvent);

private:
	FSlateApplication();

	/** 현재 Slate를 공유 소유한다. 외부 소유자가 남으면 Shutdown 뒤에도 메모리는 유지된다. */
	static TSharedPtr<FSlateApplication> currentApplication;

	/** 실제 OS 창과 메시지를 처리하는 플랫폼을 공유 소유한다. Create에서 Slate 생성 전에 연결한다. */
	static TSharedPtr<FGenericApplication> platformApplication;
};
