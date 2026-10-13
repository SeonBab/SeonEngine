#pragma once

////////////////////////////////////////////////////////////////////////////////
// 입력 이벤트의 처리 여부를 표현한다. 키의 눌림 상태는 보관하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class FReply
{
public:
	/** 이벤트를 처리했음을 나타내는 결과를 만든다. */
	[[nodiscard]] static FReply Handled() { return FReply(true); }

	/** 이벤트를 처리하지 않았음을 나타내는 결과를 만든다. */
	[[nodiscard]] static FReply Unhandled() { return FReply(false); }

	/** 이벤트를 처리했으면 true다. */
	bool IsEventHandled() const { return bIsHandled; }

private:
	explicit FReply(bool inIsHandled)
		: bIsHandled(inIsHandled)
	{
	}

	bool bIsHandled;
};
