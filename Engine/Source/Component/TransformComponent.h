#pragma once

#include "Component.h"
#include "Core/Math/EulerAngles.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector3.h"

class TransformComponent : public Component
{
public:
	/** 부모 기준(부모가 없으면 월드) 위치, 회전, 크기 묶음의 복사본을 반환한다. 행렬이 필요하면 ToMatrix()를 호출한다. */
	Transform GetTransform() const;

	/** 위치, 회전, 크기를 한 번에 부모 기준으로 설정한다. 회전은 유한한 단위 쿼터니언이어야 하며 그대로 저장한다. */
	void SetTransform(const Transform& newTransform);

	/**
	 * 부모들의 변환을 모두 합친 월드 행렬을 계산해 반환한다. 행 벡터 규칙을 따른다.
	 * 부를 때마다 부모를 따라 올라가며 계산한다. 부모가 없으면 GetTransform().ToMatrix()와 같다.
	 */
	Matrix4x4 GetWorldMatrix() const;

	/**
	 * 부모들의 변환을 모두 합친 월드 위치, 회전, 크기를 반환한다. GetWorldMatrix()를 분해한 값이다.
	 * 부모의 크기가 축마다 다르고 자식이 회전해 있으면 찌그러짐은 담을 수 없어 근삿값이 된다.
	 */
	Transform GetWorldTransform() const;

	/** 부모 기준 위치의 복사본을 반환한다. */
	Vector3 GetPosition() const;

	void SetPosition(const Vector3& newPosition);

	/**
	 * 부모 기준 회전을 도 단위 오일러 각으로 바꿔 반환한다. 내부는 쿼터니언으로 저장하므로
	 * 설정한 값과 같은 회전을 나타내는 다른 각도일 수 있다. 범위는 Quaternion::ToEulerAngles를 따른다.
	 */
	EulerAngles GetRotation() const;

	/** 부모 기준 회전을 오일러 각으로 설정한다. 각 값은 유한해야 하며 내부에서 쿼터니언으로 바꿔 저장한다. */
	void SetRotation(const EulerAngles& newRotation);

	/**
	 * 부모들의 회전까지 합친 월드 회전으로 로컬 +Z를 회전한 단위 전방 벡터를 반환한다. 위치와 크기는 무시한다.
	 * 회전은 GetWorldTransform()에서 얻으며, 저장된 회전은 유한한 단위 쿼터니언이어야 한다.
	 */
	Vector3 GetForwardVector() const;

	/** 부모 기준 축별 배율의 복사본을 반환한다. */
	Vector3 GetScale() const;

	/**
	 * 부모 기준 축별 배율을 설정한다. 단위가 없고 1이 원래 크기다. 각 값은 유한해야 한다.
	 * 0은 해당 축으로 납작하게 만들고, 음수는 해당 축으로 뒤집는다.
	 */
	void SetScale(const Vector3& newScale);

private:
	// 부모 기준 값이다. 부모가 없으면 월드 값과 같다.
	Transform transform;
};
