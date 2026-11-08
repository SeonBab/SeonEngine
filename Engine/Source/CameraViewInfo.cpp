#include "CameraViewInfo.h"

Matrix4x4 CameraViewInfo::CalculateViewMatrix() const
{
	const Vector3 inversePosition = { -position.x, -position.y, -position.z };

	// 카메라 월드 행렬(회전 * 이동)의 역이다. 이동을 먼저 되돌린 뒤 반대 회전을 적용한다.
	return Matrix4x4::CreateTranslation(inversePosition) * Matrix4x4::CreateFromQuaternion(rotation.GetInverse());
}

bool CameraViewInfo::CalculateProjectionMatrix(float aspectRatio, Matrix4x4& outProjectionMatrix) const
{
	// 0이면 투영 공식에서 0으로 나누고, 음수면 화면 좌우가 뒤집힌다. 창이 최소화되면 높이가 0이 될 수 있다.
	if (aspectRatio <= 0.0f) { return false; }
	if (nearClipDistance <= 0.0f || farClipDistance <= nearClipDistance) { return false; }

	if (projectionMode == CameraProjectionMode::Orthographic)
	{
		if (orthographicHeight <= 0.0f) { return false; }

		outProjectionMatrix = Matrix4x4::CreateOrthographicViewSize(orthographicHeight * aspectRatio, orthographicHeight, nearClipDistance, farClipDistance);
		return true;
	}

	if (verticalFovDegrees <= 0.0f || verticalFovDegrees >= 180.0f) { return false; }

	outProjectionMatrix = Matrix4x4::CreatePerspectiveFieldOfView(verticalFovDegrees, aspectRatio, nearClipDistance, farClipDistance);
	return true;
}
