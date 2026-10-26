#include "EditorEngine.h"

#include "Renderer/ImGuiRendererBackend.h"
#include "Renderer/Renderer.h"
#include "World.h"

#pragma region EditorUI

bool UEditorEngine::InitializeEditorUI(HWND windowHandle, IRenderer& renderer)
{
	return editorUI.Initialize(windowHandle, renderer.CreateImGuiRendererBackend());
}

bool UEditorEngine::HandleWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	return editorUI.HandleWindowMessage(windowHandle, message, wParam, lParam, outResult);
}

#pragma endregion

#pragma region Execution

bool UEditorEngine::Tick(float, const FInputState&, IRenderer& renderer, int width, int height)
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
	if (!world->CaptureScenes(renderer))
	{
		return false;
	}
	if (!viewportClient.Draw(world->GetScene(), renderer, width, height))
	{
		return false;
	}
	// 현재 장면 그리기는 BeginFrame이 연결한 BackBuffer를 유지하므로 UI도 같은 출력 대상에 제출한다.
	if (!editorUI.Render())
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
