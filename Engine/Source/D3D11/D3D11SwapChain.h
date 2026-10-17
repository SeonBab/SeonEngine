#pragma once

#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

/** 단일 창의 스왑 체인과 백 버퍼 RTV 참조를 소유한다. 창과 장치는 호출자가 유지한다. */
class FD3D11SwapChain
{
public:
	FD3D11SwapChain() = default;

	FD3D11SwapChain(const FD3D11SwapChain&) = delete;
	FD3D11SwapChain& operator=(const FD3D11SwapChain&) = delete;

	/** 창 모드 RGBA8·버퍼 2개의 스왑 체인과 백 버퍼 RTV를 만든다. 중복·잘못된 인자·API 실패는 false다. */
	bool Initialize(ID3D11Device* device, HWND window);

	/** RTV 다음 스왑 체인 참조를 해제한다. 빈 상태에서도 호출할 수 있다. */
	void Shutdown();

	/** 비소유 백 버퍼 RTV다. 참조를 추가하지 않으며 Resize/Shutdown/소멸 뒤 이전 포인터를 사용하면 안 된다. */
	ID3D11RenderTargetView* GetBackBufferRenderTargetView() const { return renderTargetView.Get(); }

	/** 준비된 스왑 체인을 수직 동기화 간격 1로 표시 요청한다. API 실패는 로그와 false로 전달한다. */
	bool Present();

	/**
	 * 같은 장치로 양수 크기를 적용한다.
	 * NOTE: 호출 전에 백 버퍼 바인딩을 해제해야 하며 실패 후에는 그리지 않고 Shutdown한다.
	 */
	bool Resize(ID3D11Device* device, int width, int height);

private:
	/** 지정한 체인의 백 버퍼 RTV를 만든다. 성공한 참조만 출력으로 옮긴다. */
	bool CreateBackBufferRenderTargetView(ID3D11Device* device, IDXGISwapChain1* sourceSwapChain, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& outView);

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
};
