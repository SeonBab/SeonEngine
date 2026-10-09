#pragma once

#include "CoreTypes.h"
#include "Platform/Windows/WindowsHWrapper.h"

#include <d3d11.h>
#include <wrl/client.h>

////////////////////////////////////////////////////////////////////////////////
// D3D11 장치와 즉시 컨텍스트를 관리한다. 창 출력 자원은 Viewport가 관리한다.
// 장치와 컨텍스트의 종료 책임이 하나여야 하므로 복사와 이동을 금지한다.
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
	 * D3D11 장치와 즉시 컨텍스트를 준비한다. 장치가 이미 있으면 재사용한다.
	 * 정상 반환 시 장치가 준비돼 있다. 생성 실패는 부분 자원 정리 후 치명 종료한다.
	 */
	void InitD3DDevice();

	/**
	 * 컨텍스트 바인딩과 보유 COM 참조를 정리한다. GPU 완료를 기다리는 함수는 아니다.
	 * 정상 종료 전에 장치를 사용하는 모든 Viewport와 다른 GPU 자원을 정리해야 한다.
	 * 기본 소멸자는 COM 참조만 해제하므로 정상 종료의 명시적 호출을 대신하지 않는다.
	 */
	void Shutdown();

	/**
	 * 준비된 즉시 컨텍스트로 지정한 RTV 전체를 RGBA 색상으로 지운다. 화면 제출은 하지 않는다.
	 * 장치 준비 후 호출하며 유효한 RTV와 float 네 개의 색상 배열이 필요하다.
	 * RTV와 색상은 호출 동안만 빌려 쓰며 소유하거나 해제하지 않는다.
	 */
	void ClearRenderTargetView(ID3D11RenderTargetView* InRenderTargetView, const float InClearColor[4]);

	/** 준비된 장치의 비소유 포인터를 반환한다. 생성 전과 정리 후에는 nullptr이다. */
	ID3D11Device* GetDevice() const { return Direct3DDevice.Get(); }

	/**
	 * 보관한 즉시 컨텍스트의 비소유 포인터를 반환한다. 준비 전과 종료 후에는 nullptr이다.
	 * 반환 주소는 장치가 컨텍스트 참조를 해제하기 전까지만 사용한다.
	 */
	ID3D11DeviceContext* GetDeviceContext() const;

private:
	Microsoft::WRL::ComPtr<ID3D11Device> Direct3DDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Direct3DDeviceIMContext;
};
