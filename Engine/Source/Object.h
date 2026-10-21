#pragma once

/** UObject 계열의 최소 기반이다. GC, 리플렉션, 객체 등록은 제공하지 않는다. */
class UObject
{
public:
	virtual ~UObject() = default;
};
