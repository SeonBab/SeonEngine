#include "ViewportClient.h"

#include "Renderer/Scene.h"

bool FViewportClient::Draw(FScene& scene, IRenderer& renderer, const FRenderView& renderView) const
{
	return scene.Draw(renderer, renderView);
}
