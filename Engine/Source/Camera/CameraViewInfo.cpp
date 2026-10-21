#include "CameraViewInfo.h"

#include <cmath>
#include <numbers>

FMatrix4 FCameraViewInfo::GetViewMatrix() const
{
	return worldTransform.ToMatrixNoScale().Inverse();
}

FMatrix4 FCameraViewInfo::GetProjectionMatrix(float aspectRatio) const
{
	if (projectionMode == ECameraProjectionMode::Orthographic)
	{
		const float height = orthographicWidth / aspectRatio;
		const float depthScale = 1.0f / (farClipDistance - nearClipDistance);

		FMatrix4 projection;
		projection.m[0][0] = 2.0f / orthographicWidth;
		projection.m[1][1] = 2.0f / height;
		projection.m[2][2] = depthScale;
		projection.m[3][2] = -nearClipDistance * depthScale;
		projection.m[3][3] = 1.0f;

		return projection;
	}

	const float halfFovRadians = horizontalFovDegrees * std::numbers::pi_v<float> / 360.0f;
	const float xScale = 1.0f / std::tan(halfFovRadians);
	const float depthScale = farClipDistance / (farClipDistance - nearClipDistance);

	FMatrix4 projection;
	projection.m[0][0] = xScale;
	projection.m[1][1] = xScale * aspectRatio;
	projection.m[2][2] = depthScale;
	projection.m[2][3] = 1.0f;
	projection.m[3][2] = -nearClipDistance * depthScale;

	return projection;
}

FMatrix4 FCameraViewInfo::GetWorldToClipMatrix(float aspectRatio) const
{
	return GetViewMatrix() * GetProjectionMatrix(aspectRatio);
}
