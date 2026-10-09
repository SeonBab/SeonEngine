#pragma once

////////////////////////////////////////////////////////////////////////////////
// 자체 객체 계열의 기반이다.
// GC와 리플렉션은 아직 제공하지 않는다.
////////////////////////////////////////////////////////////////////////////////
class UObject
{
public:
	/**
	 * 소속만 저장한다. 실행 초기화는 하지 않는다.
	 * @param inOuter nullptr이면 소속이 없다. 조회 / 사용 동안 대상이 살아 있어야 한다.
	 */
	explicit UObject(UObject* inOuter = nullptr);

	/** 기반 포인터를 통한 파괴를 지원한다. Outer 대상 파괴나 Shutdown 호출은 하지 않는다. */
	virtual ~UObject() = default;

	UObject(const UObject&) = delete;
	UObject& operator=(const UObject&) = delete;
	UObject(UObject&&) = delete;
	UObject& operator=(UObject&&) = delete;

	/**
	 * 비소유 소속 포인터를 반환한다. 대상의 수명이나 타입은 검사하지 않는다.
	 * @return 소속이 없으면 nullptr. const 객체에서도 소속 대상은 수정할 수 있다.
	 */
	UObject* GetOuter() const { return outer; }

private:
	// Outer는 논리적 소속을 나타내며 대상을 소유하지 않는다.
	UObject* outer;
};
