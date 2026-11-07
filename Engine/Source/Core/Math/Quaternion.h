#pragma once

struct EulerAngles;

/**
 * 회전을 나타내는 단위 쿼터니언이다. x, y, z는 회전축 방향 성분이고 w는 회전 각도의 절반에 대한 cos이다.
 * 기본값은 회전 없음이다.
 */
struct Quaternion
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	/** 오일러 각으로 만든다. EulerAngles와 같이 roll(Z), pitch(X), yaw(Y) 순서로 적용한다. */
	static Quaternion CreateFromEulerAngles(const EulerAngles& angles);

	/**
	 * 도 단위 오일러 각으로 바꾼다. pitch는 -90~90, yaw와 roll은 -180~180 범위로 나오므로
	 * CreateFromEulerAngles에 넣은 값과 같은 회전을 나타내는 다른 각도일 수 있다.
	 * pitch가 90도나 -90도에 가까우면 yaw와 roll을 구분할 수 없어 roll을 0으로 두고 남은 회전을 yaw로 돌려준다.
	 */
	EulerAngles ToEulerAngles() const;

	/** 반대 회전을 반환한다. 단위 쿼터니언이라 켤레(x, y, z의 부호를 뒤집은 값)와 같다. */
	Quaternion GetInverse() const;
};
