#include "EditorUI.h"

#include "Core/Logging/Log.h"

#include <imgui.h>
#include <imgui_impl_win32.h>

FEditorUI::~FEditorUI()
{
	Shutdown();
}

bool FEditorUI::Initialize(HWND windowHandle)
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

	IMGUI_CHECKVERSION();
	context = ImGui::CreateContext();
	if (context == nullptr)
	{
		WriteLog("EditorUI", ELogLevel::Error, "Creating the ImGui context failed.");
		return false;
	}

	if (!ImGui_ImplWin32_Init(windowHandle))
	{
		WriteLog("EditorUI", ELogLevel::Error, "Initializing the ImGui Win32 backend failed.");
		Shutdown();
		return false;
	}
	bWin32Initialized = true;

	return true;
}

void FEditorUI::Shutdown()
{
	if (context == nullptr) { return; }

	// 백엔드는 current context를 사용하므로 소유 대상을 선택하고 정리 후 기존 선택을 복원한다.
	ImGuiContext* previousContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(context);
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
