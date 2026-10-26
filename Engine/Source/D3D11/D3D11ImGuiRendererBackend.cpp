#include "D3D11ImGuiRendererBackend.h"

#include "Core/Logging/Log.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>

FD3D11ImGuiRendererBackend::FD3D11ImGuiRendererBackend(ID3D11Device* inDevice, ID3D11DeviceContext* inDeviceContext)
{
	device = inDevice;
	deviceContext = inDeviceContext;
}

FD3D11ImGuiRendererBackend::~FD3D11ImGuiRendererBackend()
{
	Shutdown();
}

bool FD3D11ImGuiRendererBackend::Initialize(ImGuiContext* inContext)
{
	if (context || !inContext || !device || !deviceContext) { return false; }

	ImGuiContext* previousContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(inContext);
	if (ImGui::GetIO().BackendRendererUserData != nullptr)
	{
		ImGui::SetCurrentContext(previousContext);
		return false;
	}
	if (!ImGui_ImplDX11_Init(device, deviceContext))
	{
		WriteLog("ImGuiRenderer", ELogLevel::Error, "Initializing the ImGui DX11 backend failed.");
		ImGui::SetCurrentContext(previousContext);
		return false;
	}
	context = inContext;
	bDX11BackendInitialized = true;
	if (!ImGui_ImplDX11_CreateDeviceObjects())
	{
		WriteLog("ImGuiRenderer", ELogLevel::Error, "Creating the ImGui DX11 device objects failed.");
		Shutdown();
		ImGui::SetCurrentContext(previousContext);
		return false;
	}

	ImGui::SetCurrentContext(previousContext);
	return true;
}

bool FD3D11ImGuiRendererBackend::NewFrame()
{
	if (!context || !bDX11BackendInitialized) { return false; }
	ImGuiContext* previousContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(context);
	ImGui_ImplDX11_NewFrame();
	ImGui::SetCurrentContext(previousContext);
	return true;
}

bool FD3D11ImGuiRendererBackend::RenderDrawData(ImDrawData* drawData)
{
	if (!context || !bDX11BackendInitialized || !drawData) { return false; }
	ImGuiContext* previousContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(context);
	ImGui_ImplDX11_RenderDrawData(drawData);
	ImGui::SetCurrentContext(previousContext);
	return true;
}

void FD3D11ImGuiRendererBackend::Shutdown()
{
	if (!context) { return; }
	ImGuiContext* previousContext = ImGui::GetCurrentContext();
	ImGui::SetCurrentContext(context);
	if (bDX11BackendInitialized)
	{
		ImGui_ImplDX11_Shutdown();
		bDX11BackendInitialized = false;
	}
	context = nullptr;
	ImGui::SetCurrentContext(previousContext);
}
