#include "EditorEngine.h"

#include "Renderer/Renderer.h"
#include "World.h"

bool UEditorEngine::InitializeEditorUI(HWND windowHandle)
{
	return editorUI.Initialize(windowHandle);
}

bool UEditorEngine::Tick(float, const FInputState&, FRenderer& renderer, int width, int height)
{
	if (!world || width <= 0 || height <= 0) { return false; }

	if (!world->SendAllEndOfFrameUpdates())
	{
		return false;
	}
	if (!renderer.BeginFrame(width, height))
	{
		return false;
	}
	if (!viewportClient.Draw(world->GetScene(), renderer, width, height))
	{
		return false;
	}

	return renderer.EndFrame();
}

void UEditorEngine::PreExit()
{
	editorUI.Shutdown();
	world.reset();
}
