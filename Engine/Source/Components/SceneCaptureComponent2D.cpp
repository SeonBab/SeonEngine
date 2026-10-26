#include "SceneCaptureComponent2D.h"

#include "Actor.h"
#include "Components/TransformComponent.h"
#include "Core/Math/Matrix4.h"
#include "Core/Math/Transform.h"
#include "Renderer/Renderer.h"
#include "Renderer/Scene.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/TextureRenderTarget2D.h"
#include "World.h"

#include <utility>

USceneCaptureComponent2D::USceneCaptureComponent2D(AActor& owner)
	: UActorComponent(owner)
{
}

void USceneCaptureComponent2D::SetTextureTarget(TSharedPtr<UTextureRenderTarget2D> newTextureTarget)
{
	textureTarget = std::move(newTextureTarget);
}

TSharedPtr<UTextureRenderTarget2D> USceneCaptureComponent2D::GetTextureTarget() const
{
	return textureTarget;
}

void USceneCaptureComponent2D::SetCaptureEveryFrame(bool enabled)
{
	bCaptureEveryFrame = enabled;
}

bool USceneCaptureComponent2D::IsCaptureEveryFrameEnabled() const
{
	return bCaptureEveryFrame;
}

ECameraProjectionMode USceneCaptureComponent2D::GetProjectionMode() const
{
	return projectionMode;
}

void USceneCaptureComponent2D::SetProjectionMode(ECameraProjectionMode newProjectionMode)
{
	projectionMode = newProjectionMode;
}

float USceneCaptureComponent2D::GetHorizontalFovDegrees() const
{
	return horizontalFovDegrees;
}

void USceneCaptureComponent2D::SetHorizontalFovDegrees(float newHorizontalFovDegrees)
{
	horizontalFovDegrees = newHorizontalFovDegrees;
}

float USceneCaptureComponent2D::GetOrthographicWidth() const
{
	return orthographicWidth;
}

void USceneCaptureComponent2D::SetOrthographicWidth(float newOrthographicWidth)
{
	orthographicWidth = newOrthographicWidth;
}

float USceneCaptureComponent2D::GetNearClipDistance() const
{
	return nearClipDistance;
}

void USceneCaptureComponent2D::SetNearClipDistance(float newNearClipDistance)
{
	nearClipDistance = newNearClipDistance;
}

float USceneCaptureComponent2D::GetFarClipDistance() const
{
	return farClipDistance;
}

void USceneCaptureComponent2D::SetFarClipDistance(float newFarClipDistance)
{
	farClipDistance = newFarClipDistance;
}

FCameraViewInfo USceneCaptureComponent2D::GetCaptureView() const
{
	const UTransformComponent* transform =
		GetOwner().GetComponent<UTransformComponent>();
	const FTransform captureTransform = transform->GetComponentTransform();

	FCameraViewInfo view;
	view.location = captureTransform.position;
	view.rotation = captureTransform.rotation.ToRotator();
	view.projectionMode = projectionMode;
	view.horizontalFovDegrees = horizontalFovDegrees;
	view.orthographicWidth = orthographicWidth;
	view.nearClipDistance = nearClipDistance;
	view.farClipDistance = farClipDistance;

	return view;
}

bool USceneCaptureComponent2D::CaptureScene(IRenderer& renderer) const
{
	if (!textureTarget ||
		textureTarget->GetWidth() <= 0 ||
		textureTarget->GetHeight() <= 0)
	{
		return false;
	}

	const float aspectRatio =
		static_cast<float>(textureTarget->GetWidth()) /
		static_cast<float>(textureTarget->GetHeight());

	const FCameraViewInfo view = GetCaptureView();
	FRenderView renderView;
	renderView.worldToView = view.CalculateViewMatrix();
	renderView.viewToClip = view.CalculateProjectionMatrix(aspectRatio);

	if (!renderer.BeginRenderTarget(*textureTarget)) { return false; }
	if (!GetOwner().GetWorld().GetScene().Draw(renderer, renderView)) { return false; }

	return renderer.EndRenderTarget();
}
