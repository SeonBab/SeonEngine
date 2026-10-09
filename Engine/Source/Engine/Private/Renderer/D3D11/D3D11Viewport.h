#pragma once

#include "CoreTypes.h"
#include "Platform/Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

class FD3D11Device;

////////////////////////////////////////////////////////////////////////////////
// 창별 스왑체인과 백버퍼 RTV를 관리한다.
// 장치는 비소유로 연결하며 이 객체보다 오래 살아 있어야 한다.
// FRenderer가 생성 / 정리 / 크기 변경 / 화면 제출을 연결한다.
////////////////////////////////////////////////////////////////////////////////
class FD3D11Viewport
{
public:
	explicit FD3D11Viewport(FD3D11Device& InDevice);
	~FD3D11Viewport();

	FD3D11Viewport(const FD3D11Viewport&) = delete;
	FD3D11Viewport& operator=(const FD3D11Viewport&) = delete;
	FD3D11Viewport(FD3D11Viewport&&) = delete;
	FD3D11Viewport& operator=(FD3D11Viewport&&) = delete;

	/**
	 * 준비된 장치를 이용해 창의 스왑체인과 백버퍼 뷰를 생성한다.
	 * 스왑체인과 백버퍼 뷰 참조가 빈 상태에서 한 번 호출한다.
	 * @param InWindowHandle 생성과 정상 종료 동안 살아 있는 Windows 창의 핸들.
	 * @param InSizeX 0보다 큰 출력 가로 픽셀 수.
	 * @param InSizeY 0보다 큰 출력 세로 픽셀 수.
	 * @return 생성 성공 시 true. 실패하면 이 함수에서 확보한 자원을 정리하고 false.
	 */
	[[nodiscard]] bool Init(void* InWindowHandle, uint32 InSizeX, uint32 InSizeY);

	/**
	 * 보관한 백버퍼 뷰 참조를 먼저 해제하고 스왑체인 참조를 해제한다. 장치 종료나 GPU 완료 대기는 하지 않는다.
	 * 호출자는 먼저 컨텍스트의 출력 바인딩과 외부 백버퍼 참조를 정리하고 OS 창을 유지한다.
	 */
	void Shutdown();

	/**
	 * 스왑체인의 비소유 포인터를 반환한다. 생성 전과 정리 후에는 nullptr이다.
	 * 반환된 포인터는 Viewport가 해당 참조를 해제하기 전까지만 사용한다.
	 */
	IDXGISwapChain* GetSwapChain() const;

	/**
	 * 백버퍼 뷰의 비소유 포인터를 반환한다. 뷰 생성 전 / 해제 후에는 nullptr이다.
	 * 반환 주소는 뷰의 참조를 해제하거나 재생성하기 전까지만 사용한다.
	 */
	ID3D11RenderTargetView* GetBackBufferRenderTargetView() const;

	/**
	 * 창의 출력 버퍼와 뷰를 새 크기에 맞춘다. 최소화 상태에서는 호출하지 않는다.
	 * Init 성공 후 프레임 출력 전에 호출한다. 외부 백버퍼 참조는 먼저 정리해야 한다.
	 * 컨텍스트 상태를 초기화하므로 다음 그리기에서 필요한 바인딩을 다시 설정해야 한다. 출력 재구성 실패 시 치명 종료한다.
	 * @param InSizeX 0보다 큰 출력 가로 픽셀 수.
	 * @param InSizeY 0보다 큰 출력 세로 픽셀 수.
	 */
	void Resize(uint32 InSizeX, uint32 InSizeY);

	/**
	 * 그리기를 마친 백버퍼를 창에 표시한다. 삼각형을 그리는 함수는 아니다.
	 * 생성된 스왑체인이 필요하며 종료 후나 최소화 상태에서는 호출하지 않는다.
	 * 실패 원인은 Error로 기록하고 종료 / 복구 판단은 호출자가 한다.
	 * @return HRESULT가 실패 상태가 아니면 true. 실패하면 false이며 실제 화면 표시 완료를 보장하지 않는다.
	 */
	[[nodiscard]] bool Present();

private:
	/**
	 * 준비된 장치 / 스왑체인으로 빈 백버퍼 뷰 참조를 생성한다.
	 * Init에서 호출한다. 실패 시 확보한 참조를 정리하고 원인을 Error로 기록한다.
	 * @return 뷰 생성 성공 시 true, 실패 시 false. 장치 / 스왑체인은 종료하지 않는다.
	 */
	[[nodiscard]] bool CreateBackBufferRenderTargetView();

	FD3D11Device* const device;
	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferRenderTargetView;

	// 요청 크기가 아니라 출력 자원 준비에 성공한 크기다. 종료하면 0으로 돌아간다.
	uint32 SizeX = 0;
	uint32 SizeY = 0;
};
