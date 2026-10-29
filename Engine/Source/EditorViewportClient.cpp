#include "EditorViewportClient.h"

#include "Renderer/Renderer.h"
#include "Renderer/Scene.h"
#include "Renderer/SceneRenderData.h"

FVector3 FViewportCameraTransform::GetLocation() const
{
	return viewLocation;
}

void FViewportCameraTransform::SetLocation(FVector3 newLocation)
{
	viewLocation = newLocation;
}

FRotator FViewportCameraTransform::GetRotation() const
{
	return viewRotation;
}

void FViewportCameraTransform::SetRotation(FRotator newRotation)
{
	viewRotation = newRotation;
}

#pragma region View

FViewportCameraTransform& FEditorViewportClient::GetViewTransform()
{
	return projectionMode == ECameraProjectionMode::Perspective ? viewTransformPerspective : viewTransformOrthographic;
}

const FViewportCameraTransform& FEditorViewportClient::GetViewTransform() const
{
	return projectionMode == ECameraProjectionMode::Perspective ? viewTransformPerspective : viewTransformOrthographic;
}

FCameraViewInfo FEditorViewportClient::GetCameraView() const
{
	const FViewportCameraTransform& transform = GetViewTransform();

	FCameraViewInfo view;
	view.location = transform.GetLocation();
	view.rotation = transform.GetRotation();
	view.projectionMode = projectionMode;
	view.horizontalFovDegrees = horizontalFovDegrees;
	view.orthographicWidth = orthographicWidth;
	view.nearClipDistance = nearClipDistance;
	view.farClipDistance = farClipDistance;

	return view;
}

#pragma endregion

#pragma region Projection

ECameraProjectionMode FEditorViewportClient::GetProjectionMode() const
{
	return projectionMode;
}

void FEditorViewportClient::SetProjectionMode(ECameraProjectionMode newProjectionMode)
{
	projectionMode = newProjectionMode;
}

float FEditorViewportClient::GetHorizontalFovDegrees() const
{
	return horizontalFovDegrees;
}

void FEditorViewportClient::SetHorizontalFovDegrees(float newHorizontalFovDegrees)
{
	horizontalFovDegrees = newHorizontalFovDegrees;
}

float FEditorViewportClient::GetOrthographicWidth() const
{
	return orthographicWidth;
}

void FEditorViewportClient::SetOrthographicWidth(float newOrthographicWidth)
{
	orthographicWidth = newOrthographicWidth;
}

float FEditorViewportClient::GetNearClipDistance() const
{
	return nearClipDistance;
}

void FEditorViewportClient::SetNearClipDistance(float newNearClipDistance)
{
	nearClipDistance = newNearClipDistance;
}

float FEditorViewportClient::GetFarClipDistance() const
{
	return farClipDistance;
}

void FEditorViewportClient::SetFarClipDistance(float newFarClipDistance)
{
	farClipDistance = newFarClipDistance;
}

#pragma endregion

#pragma region RenderTarget

bool FEditorViewportClient::ResizeRenderTarget(IRenderer& renderer, int32 width, int32 height)
{
	if (width <= 0 || height <= 0) { return false; }

	if (renderTarget.GetWidth() == width && renderTarget.GetHeight() == height)
	{
		return true;
	}

	return renderer.CreateRenderTarget2D(width, height, renderTarget);
}

const UTextureRenderTarget2D& FEditorViewportClient::GetRenderTarget() const
{
	return renderTarget;
}

void FEditorViewportClient::ReleaseRenderTarget()
{
	renderTarget.SetRenderData(nullptr);
}

#pragma endregion

#pragma region Drawing

bool FEditorViewportClient::Draw(FScene& scene, IRenderer& renderer) const
{
	const int32 width = renderTarget.GetWidth();
	const int32 height = renderTarget.GetHeight();
	if (width <= 0 || height <= 0) { return false; }

	const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	const FCameraViewInfo view = GetCameraView();
	FRenderView renderView;
	renderView.worldToView = view.CalculateViewMatrix();
	renderView.viewToClip = view.CalculateProjectionMatrix(aspectRatio);
	renderView.location = view.location;
	renderView.bOrthographic = view.projectionMode == ECameraProjectionMode::Orthographic;

	if (!renderer.BeginRenderTarget(renderTarget)) { return false; }

	const bool drawn = scene.Draw(renderer, renderView);
	const bool restored = renderer.EndRenderTarget();

	return drawn && restored;
}

#pragma endregion
