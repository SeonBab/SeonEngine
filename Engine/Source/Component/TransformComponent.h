#pragma once

#include "Component.h"
#include "Core/Vector3.h"

class TransformComponent : public Component
{
public:
	/** 월드 위치의 복사본을 반환한다. */
	Vector3 GetPosition() const;

	void SetPosition(const Vector3& newPosition);

private:
	Vector3 position;
};
