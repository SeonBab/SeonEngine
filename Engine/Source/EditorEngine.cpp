#include "EditorEngine.h"

#include "Core/Logging/Log.h"
#include "Maps/GameMapsSettings.h"
#include "Renderer/ImGuiRendererBackend.h"
#include "Renderer/Renderer.h"
#include "World.h"

#pragma region Lifecycle

UEditorEngine::~UEditorEngine()
{
	PreExit();
}

#pragma endregion

#pragma region EditorUI

bool UEditorEngine::InitializeEditorUI(HWND windowHandle, IRenderer& renderer)
{
	if (bEditorUIInitialized) { return false; }
	if (UEngine::Get() != this || !world)
	{
		WriteLog("Editor", ELogLevel::Error, "Initializing editor UI requires this engine's common services.");
		return false;
	}
	if (!editorUI.Initialize(windowHandle, renderer.CreateImGuiRendererBackend()))
	{
		PreExit();
		return false;
	}
	bEditorUIInitialized = true;

	return true;
}

bool UEditorEngine::HandleWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	return editorUI.HandleWindowMessage(windowHandle, message, wParam, lParam, outResult);
}

#pragma endregion

#pragma region Execution

bool UEditorEngine::Initialize(IRenderer& renderer)
{
	if (!UEngine::Initialize(renderer)) { return false; }

	if (!LoadMap(GetGameMapsSettings().editorStartupMap) || !world->CreateRenderStates())
	{
		PreExit();
		return false;
	}

	return true;
}

bool UEditorEngine::Tick(float, const FInputState&, IRenderer& renderer, int width, int height)
{
	if (!world || width <= 0 || height <= 0) { return false; }

	if (!world->SendAllEndOfFrameUpdates())
	{
		return false;
	}
	int32 viewportWidth = 0;
	int32 viewportHeight = 0;
	if (!editorUI.BeginFrame() || !editorUI.Build(viewportWidth, viewportHeight))
	{
		return false;
	}
	if (viewportWidth > 0 && viewportHeight > 0 && !viewportClient.ResizeRenderTarget(renderer, viewportWidth, viewportHeight))
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
	if (viewportWidth > 0 && viewportHeight > 0 && !viewportClient.Draw(world->GetScene(), renderer))
	{
		return false;
	}
	if (viewportWidth > 0 && viewportHeight > 0 && !editorUI.BuildViewportImage(viewportClient.GetRenderTarget()))
	{
		return false;
	}
	// 타겟을 읽는 이미지를 포함한 UI를 복원된 BackBuffer에 제출한다. 접힌 패널은 장면과 이미지를 생략한다.
	if (!editorUI.EndFrame())
	{
		return false;
	}

	return renderer.EndFrame();
}

void UEditorEngine::PreExit()
{
	bEditorUIInitialized = false;
	editorUI.Shutdown();
	viewportClient.ReleaseRenderTarget();
	UEngine::PreExit();
}

#pragma endregion
