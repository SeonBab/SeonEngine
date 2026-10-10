#pragma once

#include "Templates/UniquePtr.h"

class IModuleInterface;

////////////////////////////////////////////////////////////////////////////////
// 정적으로 연결할 모듈 하나를 등록·소유한다. 등록 후 준비 / 종료를 명시적으로 호출한다.
////////////////////////////////////////////////////////////////////////////////
class FModuleManager
{
public:
	/** 빈 모듈 소유 자리를 준비한다. 모듈 생성이나 StartupModule 호출은 하지 않는다. */
	FModuleManager();
	/** 소유 모듈 객체를 해제한다. ShutdownModule을 자동 호출하지 않는다. */
	~FModuleManager();

	FModuleManager(const FModuleManager&) = delete;
	FModuleManager& operator=(const FModuleManager&) = delete;
	FModuleManager(FModuleManager&&) = delete;
	FModuleManager& operator=(FModuleManager&&) = delete;

	/** 성공 시에만 모듈 소유권을 넘겨받는다. 빈 입력 / 중복 등록은 false이며 입력과 기존 모듈을 유지한다. StartupModule은 호출하지 않는다. */
	[[nodiscard]] bool RegisterModule(TUniquePtr<IModuleInterface>& inModule);

	/**
	 * 등록된 모듈의 StartupModule을 호출하고 비소유 포인터를 반환한다. 미등록이면 nullptr다.
	 * 호출자는 등록 후 한 번만 호출해야 한다. 중복 / 재진입은 검사하지 않는다.
	 * 반환 포인터는 관리자가 모듈을 소유하는 동안만 유효하며 준비 성공 여부를 별도로 판정하지 않는다.
	 * 객체 생성 / DLL 로딩은 하지 않는다.
	 */
	[[nodiscard]] IModuleInterface* LoadModule();

	/**
	 * ShutdownModule 호출 후 소유 모듈을 파괴한다. 미등록이면 false, 정리 후 true다.
	 * 호출자는 RegisterModule / LoadModule / UnloadModule 순서를 지키고 훅 중 재진입하지 않아야 한다.
	 * 해제 후 기존 비소유 포인터는 무효다. DLL 언로드 / 준비 상태 검사는 하지 않는다.
	 */
	[[nodiscard]] bool UnloadModule();

private:
	TUniquePtr<IModuleInterface> module;
};
