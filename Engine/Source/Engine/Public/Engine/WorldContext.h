#pragma once

#include "Templates/UniquePtr.h"

class FWorld;
class UEngine;
class UGameInstance;

////////////////////////////////////////////////////////////////////////////////
// 엔진 내부에서 현재 월드와 게임 세션의 연결을 관리한다.
// 월드는 소유하지만 GameInstance는 소유하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FWorldContext
{
	// 생성 / 정리 담당만 소유 관계와 세션 연결을 변경한다.
	friend class UEngine;
	friend class UGameInstance;

public:
	FWorldContext();
	~FWorldContext();

	FWorldContext(const FWorldContext&) = delete;
	FWorldContext& operator=(const FWorldContext&) = delete;
	FWorldContext(FWorldContext&&) = delete;
	FWorldContext& operator=(FWorldContext&&) = delete;

	/** 현재 월드를 비소유 반환한다. 아직 월드가 없으면 nullptr다. */
	FWorld* World() const { return currentWorld.get(); }

private:
	TUniquePtr<FWorld> currentWorld;
	UGameInstance* owningGameInstance = nullptr;
};
