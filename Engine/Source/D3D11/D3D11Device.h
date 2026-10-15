#pragma once

#include "Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

/** D3D11 장치와 즉시 컨텍스트를 소유한다. COM 참조는 명시적 정리 또는 소멸 시 해제한다. */
class FD3D11Device
{
public:
	FD3D11Device() = default;

	FD3D11Device(const FD3D11Device&) = delete;
	FD3D11Device& operator=(const FD3D11Device&) = delete;

	/** 하드웨어 장치를 준비한다. 이미 준비됐거나 생성에 실패하면 false다. */
	bool Initialize();

	/** 즉시 컨텍스트·장치 순서로 참조를 해제한다. 빈 상태에서도 호출할 수 있다. */
	void Shutdown();

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
};
