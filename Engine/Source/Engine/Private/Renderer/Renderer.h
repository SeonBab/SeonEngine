#pragma once

#include "Renderer/D3D11/D3D11Device.h"
#include "Renderer/D3D11/D3D11Viewport.h"
#include "Renderer/RendererInterface.h"

////////////////////////////////////////////////////////////////////////////////
// SeonEngine의 기본 렌더러다. 현재 D3D11 구현으로 단일 창에 출력한다.
// 공통 계약을 구현하며 장치 준비 / 명령은 Device, 창 출력은 Viewport에 맡긴다.
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

	/** 장치와 창 출력을 준비한다. 출력 생성 실패 시 부분 출력과 장치를 정리하고 false를 반환한다. */
	[[nodiscard]] bool Init(void* WindowHandle, uint32 SizeX, uint32 SizeY) override;

	/** OS 창이 살아 있는 동안 바인딩 / 창 출력 / 장치 순서로 정리한다. 기본 소멸자는 명시적 종료를 대신하지 않는다. */
	void Shutdown() override;

	/** 초기화 성공 후 최소화가 아닌 상태의 양수 클라이언트 크기를 전달한다. */
	void Resize(uint32 SizeX, uint32 SizeY) override;

	/** 필요한 크기 변경을 적용한 뒤 호출하며 최소화 / 크기 0에서는 호출하지 않는다. */
	void RenderFrame() override;

private:
	// 현재는 D3D11 고정 구성이다. RHI 도입 시 API 독립 자원 / 명령 계약으로 교체한다.
	FD3D11Device Device;
	FD3D11Viewport Viewport{Device};
};
