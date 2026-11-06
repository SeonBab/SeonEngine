#pragma once

#include "Component.h"
#include "Core/EulerAngles.h"
#include "Core/Transform.h"
#include "Core/Vector3.h"

class TransformComponent : public Component
{
public:
	/** 위치, 회전, 크기 묶음의 복사본을 반환한다. 행렬이 필요하면 ToMatrix()를 호출한다. */
	Transform GetTransform() const;

	/** 월드 위치의 복사본을 반환한다. */
	Vector3 GetPosition() const;

	void SetPosition(const Vector3& newPosition);

	/**
	 * 월드 기준 회전을 도 단위 오일러 각으로 바꿔 반환한다. 내부는 쿼터니언으로 저장하므로
	 * 설정한 값과 같은 회전을 나타내는 다른 각도일 수 있다. 범위는 Quaternion::ToEulerAngles를 따른다.
	 */
	EulerAngles GetRotation() const;

	/** 월드 기준 회전을 오일러 각으로 설정한다. 각 값은 유한해야 하며 내부에서 쿼터니언으로 바꿔 저장한다. */
	void SetRotation(const EulerAngles& newRotation);

	/** 월드 기준 축별 배율의 복사본을 반환한다. */
	Vector3 GetScale() const;

	/**
	 * 월드 기준 축별 배율을 설정한다. 단위가 없고 1이 원래 크기다. 각 값은 유한해야 한다.
	 * 0은 해당 축으로 납작하게 만들고, 음수는 해당 축으로 뒤집는다.
	 */
	void SetScale(const Vector3& newScale);

private:
	// 부모 관계가 없으므로 월드 기준 값이다.
	Transform transform;
};
