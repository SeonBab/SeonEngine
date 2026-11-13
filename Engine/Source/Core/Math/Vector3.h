#pragma once

struct Vector3
{
	/** 각 성분의 합을 반환하며 두 피연산자는 바꾸지 않는다. */
	Vector3 operator+(const Vector3& other) const
	{
		return Vector3{ x + other.x, y + other.y, z + other.z };
	}

	/** 각 성분의 차이를 반환하며 두 피연산자는 바꾸지 않는다. */
	Vector3 operator-(const Vector3& other) const
	{
		return Vector3{ x - other.x, y - other.y, z - other.z };
	}

	/** 각 성분에 scalar를 곱한 새 벡터를 반환한다. */
	Vector3 operator*(float scalar) const
	{
		return Vector3{ x * scalar, y * scalar, z * scalar };
	}

	/** 각 성분을 scalar로 나눈 새 벡터를 반환한다. scalar는 0이 아니어야 한다. */
	Vector3 operator/(float scalar) const
	{
		return Vector3{ x / scalar, y / scalar, z / scalar };
	}

	/** other의 각 성분을 현재 벡터에 더하고 자기 자신의 참조를 반환한다. */
	Vector3& operator+=(const Vector3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	/** other의 각 성분을 현재 벡터에서 빼고 자기 자신의 참조를 반환한다. */
	Vector3& operator-=(const Vector3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	/** 두 벡터의 내적을 반환한다. */
	float Dot(const Vector3& other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	/** 두 벡터의 외적을 반환한다. 입력 순서를 바꾸면 결과의 부호가 반대가 된다. */
	Vector3 Cross(const Vector3& other) const
	{
		return Vector3{ y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x };
	}

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};
