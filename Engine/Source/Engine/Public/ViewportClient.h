#pragma once

struct FInputKeyEventArgs;

/** 화면에서 게임 측으로 키 입력을 전달하는 최소 계약이다. */
class FViewportClient
{
public:
	/** 기반 포인터를 통해 파생 객체를 파괴할 수 있게 한다. */
	virtual ~FViewportClient() = default;

	/** 입력을 처리했으면 true다. 기본 구현은 false를 반환하며 이벤트 참조를 보관하지 않는다. */
	virtual bool InputKey(const FInputKeyEventArgs&)
	{
		return false;
	}
};
