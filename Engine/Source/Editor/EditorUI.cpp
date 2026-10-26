#include "EditorUI.h"

#include "Core/Logging/Log.h"
#include "Renderer/ImGuiRendererBackend.h"

#include <imgui.h>
#include <imgui_impl_win32.h>

#include <utility>

// 백엔드 헤더가 Windows 타입 의존을 피하려고 제외한 함수 선언이다.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

FEditorUI::FEditorUI() = default;

FEditorUI::~FEditorUI()
{
	Shutdown();
}

bool FEditorUI::Initialize(HWND windowHandle, TUniquePtr<IImGuiRendererBackend> newRendererBackend)
{
	if (context != nullptr || ImGui::GetCurrentContext() != nullptr)
	{
		WriteLog("EditorUI", ELogLevel::Error, "Initializing editor UI requires no existing ImGui context.");
		return false;
	}

	if (windowHandle == nullptr || !IsWindow(windowHandle))
	{
		WriteLog("EditorUI", ELogLevel::Error, "Initializing editor UI requires a valid window handle.");
		return false;
	}

	if (!newRendererBackend)
	{
		WriteLog("EditorUI", ELogLevel::Error, "Initializing editor UI requires a supported renderer backend.");
		return false;
	}

	IMGUI_CHECKVERSION();
	context = ImGui::CreateContext();
	if (context == nullptr)
	{
		WriteLog("EditorUI", ELogLevel::Error, "Creating the ImGui context failed.");
		return false;
	}

	rendererBackend = std::move(newRendererBackend);
	if (!ImGui_ImplWin32_Init(windowHandle))
	{
		WriteLog("EditorUI", ELogLevel::Error, "Initializing the ImGui Win32 backend failed.");
		Shutdown();
		return false;
	}
	bWin32Initialized = true;

	if (!rendererBackend->Initialize(context))
	{
		WriteLog("EditorUI", ELogLevel::Error, "Initializing the ImGui renderer backend failed.");
		Shutdown();
		return false;
	}

	return true;
}

bool FEditorUI::HandleWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	if (context == nullptr || !bWin32Initialized) { return false; }

	// 백엔드는 current context를 사용하므로 소유 대상을 선택하고 기존 선택을 복원한다.
	ImGuiContext* previousContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(context);
	const LRESULT result = ImGui_ImplWin32_WndProcHandler(windowHandle, message, wParam, lParam);
	ImGui::SetCurrentContext(previousContext);

	if (result == 0) { return false; }

	outResult = result;
	return true;
}

bool FEditorUI::Render()
{
	if (context == nullptr || !bWin32Initialized || !rendererBackend) { return false; }

	ImGuiContext* previousContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(context);

	if (!rendererBackend->NewFrame())
	{
		ImGui::SetCurrentContext(previousContext);
		return false;
	}
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("SeonEngine"))
	{
		ImGui::TextUnformatted("Editor UI is ready.");
	}
	ImGui::End();

	ImGui::Render();
	const bool rendered = rendererBackend->RenderDrawData(ImGui::GetDrawData());

	ImGui::SetCurrentContext(previousContext);
	return rendered;
}

void FEditorUI::Shutdown()
{
	if (context == nullptr)
	{
		rendererBackend.reset();
		return;
	}

	// 백엔드는 current context를 사용하므로 소유 대상을 선택하고 정리 후 기존 선택을 복원한다.
	ImGuiContext* previousContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(context);
	if (rendererBackend)
	{
		rendererBackend->Shutdown();
		rendererBackend.reset();
	}
	if (bWin32Initialized)
	{
		ImGui_ImplWin32_Shutdown();
		bWin32Initialized = false;
	}

	if (previousContext == context) { previousContext = nullptr; }
	ImGui::DestroyContext(context);
	context = nullptr;
	ImGui::SetCurrentContext(previousContext);
}
