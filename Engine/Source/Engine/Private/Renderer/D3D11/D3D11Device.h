#pragma once

#include "CoreTypes.h"
#include "Platform/Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

////////////////////////////////////////////////////////////////////////////////
// D3D11 장치와 단일 창의 출력 자원을 함께 관리한다. OS 창은 소유하지 않는다.
// 장치와 출력 상태의 종료 책임이 하나여야 하므로 복사와 이동을 금지한다.
////////////////////////////////////////////////////////////////////////////////
class FD3D11Device
{
public:
	FD3D11Device() = default;
	~FD3D11Device() = default;

	FD3D11Device(const FD3D11Device&) = delete;
	FD3D11Device& operator=(const FD3D11Device&) = delete;
	FD3D11Device(FD3D11Device&&) = delete;
	FD3D11Device& operator=(FD3D11Device&&) = delete;

	/**
	 * 장치와 출력 자원을 한 번 준비한다. 종료 후 재초기화는 지원하지 않는다.
	 * @param InWindowHandle 초기화와 정상 종료 동안 살아 있는 Windows 창의 핸들.
	 * @param InSizeX 0보다 큰 클라이언트 영역의 가로 픽셀 수.
	 * @param InSizeY 0보다 큰 클라이언트 영역의 세로 픽셀 수.
	 * @return 필수 자원 생성에 성공하면 true. 실패하면 확보한 자원을 정리하고 false.
	 */
	[[nodiscard]] bool Init(void* InWindowHandle, uint32 InSizeX, uint32 InSizeY);

	/**
	 * 컨텍스트 바인딩과 보유 COM 참조를 정리한다. GPU 완료를 기다리는 함수는 아니다.
	 * 정상 종료는 OS 창이 살아 있는 동안 호출한다. Init 실패 시에는 내부에서 정리한다.
	 * 기본 소멸자는 COM 참조만 해제하므로 정상 종료의 명시적 호출을 대신하지 않는다.
	 */
	void Shutdown();

	/**
	 * OS 창을 바꾸지 않고 출력 버퍼와 뷰를 새 크기에 맞춘다. 같은 크기면 생략한다.
	 * Init 성공 후 프레임 출력 전에 호출한다. 최소화 상태에서는 호출하지 않는다.
	 * 컨텍스트 상태를 초기화하므로 이후 그리기에 필요한 바인딩은 다시 설정해야 한다.
	 * 출력 자원 재구성에 실패하면 치명 종료한다.
	 * @param InSizeX 0보다 큰 클라이언트 영역의 가로 픽셀 수.
	 * @param InSizeY 0보다 큰 클라이언트 영역의 세로 픽셀 수.
	 */
	void Resize(uint32 InSizeX, uint32 InSizeY);

	/**
	 * 백버퍼를 검증용 색으로 지우고 화면에 제출한다. 제출 실패 시 치명 종료한다.
	 * Init 성공 후 필요한 크기 변경을 적용하고, 최소화가 아니며 출력 크기가 양수일 때만 호출한다.
	 */
	void RenderFrame();

private:
	Microsoft::WRL::ComPtr<ID3D11Device> Direct3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Direct3DDeviceIMContext;

	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferRenderTargetView;

	// 요청 크기가 아니라 출력 자원 준비에 성공한 크기다. 종료하면 0으로 돌아간다.
	uint32 SizeX = 0;
	uint32 SizeY = 0;
};
