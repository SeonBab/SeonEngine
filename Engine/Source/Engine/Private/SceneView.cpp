#include "SceneView.h"

FSceneViewProjectionData::FSceneViewProjectionData()
{
	viewRotationMatrix.SetIdentity();
	projectionMatrix.SetIdentity();
}

FViewMatrices::FMinimalInitializer::FMinimalInitializer()
{
	viewRotationMatrix.SetIdentity();
	projectionMatrix.SetIdentity();
}

FViewMatrices::FViewMatrices()
{
	worldToView.SetIdentity();
	viewToClip.SetIdentity();
}

FViewMatrices::FViewMatrices(const FMinimalInitializer& initializer)
	: FViewMatrices()
{
	Init(initializer);
}

FViewMatrices::FViewMatrices(const FSceneViewInitOptions& initOptions)
	: FViewMatrices()
{
	FMinimalInitializer initializer;
	initializer.viewOrigin = initOptions.viewOrigin;
	initializer.viewRotationMatrix = initOptions.viewRotationMatrix;
	initializer.projectionMatrix = initOptions.projectionMatrix;

	Init(initializer);
}

const FMatrix4& FViewMatrices::GetWorldToView() const
{
	return worldToView;
}

const FMatrix4& FViewMatrices::GetViewToClip() const
{
	return viewToClip;
}

void FViewMatrices::Init(const FMinimalInitializer& initializer)
{
	FMatrix4 translationMatrix;
	translationMatrix.SetIdentity();

	translationMatrix.m[3][0] = -initializer.viewOrigin.x;
	translationMatrix.m[3][1] = -initializer.viewOrigin.y;
	translationMatrix.m[3][2] = -initializer.viewOrigin.z;

	worldToView = translationMatrix * initializer.viewRotationMatrix;
	viewToClip = initializer.projectionMatrix;
}

FSceneView::FSceneView(const FSceneViewInitOptions& initOptions)
	: viewMatrices(initOptions)
{
}
