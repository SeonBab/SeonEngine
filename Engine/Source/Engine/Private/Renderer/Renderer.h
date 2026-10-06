#pragma once

#include "Renderer/D3D11/D3D11Device.h"
#include "Renderer/RendererInterface.h"

////////////////////////////////////////////////////////////////////////////////
// SeonEngine의 기본 렌더러다. 현재 D3D11 구현으로 단일 창에 출력한다.
// 공통 계약을 구현하며 그래픽스 API 호출은 내부 장치에 맡긴다.
////////////////////////////////////////////////////////////////////////////////
class FRenderer final : public IRenderer
{
public:
	FRenderer() = default;
	~FRenderer() override = default;

	FRenderer(const FRenderer&) = delete;
	FRenderer& operator=(const FRenderer&) = delete;
	FRenderer(FRenderer&&) = delete;
	FRenderer& operator=(FRenderer&&) = delete;

	/** 공통 초기화 계약으로 내부 장치를 준비한다. 실패 시 내부 장치가 부분 자원을 정리한다. */
	[[nodiscard]] bool Init(void* WindowHandle, uint32 SizeX, uint32 SizeY) override;

	/** OS 창이 살아 있는 동안 호출한다. 기본 소멸자는 명시적 종료를 대신하지 않는다. */
	void Shutdown() override;

	/** 초기화 성공 후 최소화가 아닌 상태의 양수 클라이언트 크기를 전달한다. */
	void Resize(uint32 SizeX, uint32 SizeY) override;

	/** 필요한 크기 변경을 적용한 뒤 호출하며 최소화 / 크기 0에서는 호출하지 않는다. */
	void RenderFrame() override;

private:
	// 현재는 D3D11 고정 구성이다. RHI 도입 시 API 독립 자원 / 명령 계약으로 교체한다.
	FD3D11Device Device;
};
