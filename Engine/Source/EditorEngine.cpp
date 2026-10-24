#include "EditorEngine.h"

#include "Renderer/Renderer.h"
#include "World.h"

#pragma region EditorUI

bool UEditorEngine::InitializeEditorUI(HWND windowHandle)
{
	return editorUI.Initialize(windowHandle);
}

bool UEditorEngine::HandleWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	return editorUI.HandleWindowMessage(windowHandle, message, wParam, lParam, outResult);
}

#pragma endregion

#pragma region Execution

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

#pragma endregion
