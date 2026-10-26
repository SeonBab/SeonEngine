#pragma once

#include "Renderer/ImGuiRendererBackend.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

/** 빌린 D3D11 장치와 즉시 컨텍스트에 ImGui DX11 backend를 연결한다. Renderer와 ImGui context보다 먼저 정리한다. */
class FD3D11ImGuiRendererBackend : public IImGuiRendererBackend
{
public:
	/** 생성만으로 ImGui를 초기화하지 않는다. device와 deviceContext는 이 객체가 정리될 때까지 유효해야 한다. */
	FD3D11ImGuiRendererBackend(ID3D11Device* inDevice, ID3D11DeviceContext* inDeviceContext);
	~FD3D11ImGuiRendererBackend() override;

	FD3D11ImGuiRendererBackend(const FD3D11ImGuiRendererBackend&) = delete;
	FD3D11ImGuiRendererBackend& operator=(const FD3D11ImGuiRendererBackend&) = delete;
	FD3D11ImGuiRendererBackend(FD3D11ImGuiRendererBackend&&) = delete;
	FD3D11ImGuiRendererBackend& operator=(FD3D11ImGuiRendererBackend&&) = delete;

	[[nodiscard]] bool Initialize(ImGuiContext* inContext) override;
	[[nodiscard]] bool NewFrame() override;
	[[nodiscard]] bool RenderDrawData(ImDrawData* drawData) override;
	void Shutdown() override;

private:
	// Renderer에서 빌린다. 초기화 성공 후 공식 ImGui backend도 종료까지 COM 참조를 유지한다.
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	// EditorUI가 소유한다. 이 backend의 Shutdown과 파괴 뒤에 context를 파괴해야 한다.
	ImGuiContext* context = nullptr;
	// Init 성공 후에만 true다. device objects 생성 실패도 준비된 backend를 역정리한다.
	bool bDX11BackendInitialized = false;
};
