#pragma once

struct Quaternion;
struct Vector3;

/**
 * 4x4 행렬이다. m[행][열]로 접근하며 행 벡터 규칙(v * M)을 쓴다.
 * 이동 성분은 마지막 행의 m[3][0], m[3][1], m[3][2]에 있고, 여러 변환은 적용 순서대로 왼쪽부터 곱한다.
 * DirectXMath의 XMFLOAT4X4와 배치가 같다. 셰이더로 보낼 때의 전치 여부는 렌더러가 정한다.
 * 기본값은 단위 행렬이다.
 */
struct Matrix4x4
{
	float m[4][4] = {
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f } };

	/** 행렬 곱셈이다. 행 벡터 규칙이므로 왼쪽 행렬의 변환을 먼저 적용한다. */
	Matrix4x4 operator*(const Matrix4x4& other) const;

	/** 크기 행렬이다. 대각선에 축별 배율을 둔다. */
	static Matrix4x4 CreateScale(const Vector3& scale);

	/** 이동 행렬이다. 마지막 행에 위치를 둔다. */
	static Matrix4x4 CreateTranslation(const Vector3& position);

	/** 단위 쿼터니언으로 회전 행렬을 만든다. 각 행은 회전된 X, Y, Z 축이다. */
	static Matrix4x4 CreateFromQuaternion(const Quaternion& rotation);

	/**
	 * 왼손 좌표계 원근 투영 행렬이다. 세로 시야각은 도 단위이고 화면 비율은 가로를 세로로 나눈 값이다.
	 * near 거리는 깊이 0, far 거리는 깊이 1이 된다. 0 < 시야각 < 180, 화면 비율 > 0, 0 < near < far를 전제로 한다.
	 */
	static Matrix4x4 CreatePerspectiveFieldOfView(float verticalFovDegrees, float aspectRatio, float nearZ, float farZ);
};
