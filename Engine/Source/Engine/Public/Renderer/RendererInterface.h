#pragma once

#include "Containers/Array.h"
#include "CoreTypes.h"

struct FMeshRenderData;

////////////////////////////////////////////////////////////////////////////////
// 엔진과 렌더러 구현 사이의 공통 계약이다.
// OS 창을 빌려 사용하며 그래픽스 자원의 수명은 구현이 관리한다.
////////////////////////////////////////////////////////////////////////////////
class IRenderer
{
public:
	virtual ~IRenderer() = default;

	/**
	 * 렌더러를 한 번 초기화한다. 종료 후 재초기화는 지원하지 않는다.
	 * 창 소유권은 플랫폼에 유지된다.
	 * 실패 전에 확보한 그래픽스 자원은 구현이 정리한다.
	 * @param WindowHandle 유효한 OS 창 핸들.
	 * @param SizeX 0보다 큰 클라이언트 영역의 가로 픽셀 수.
	 * @param SizeY 0보다 큰 클라이언트 영역의 세로 픽셀 수.
	 * @return 성공하면 true. 실패하면 엔진이 치명 종료하며 Shutdown()을 호출하지 않는다.
	 */
	[[nodiscard]] virtual bool Init(void* WindowHandle, uint32 SizeX, uint32 SizeY) = 0;

	/**
	 * 성공적으로 초기화된 렌더러의 그래픽스 자원을 해제한다.
	 * 렌더 씬이 있다면 먼저 파괴하고, OS 창이 살아 있는 동안 한 번 호출한다.
	 * 초기화 실패 시에는 호출하지 않는다. 종료 후 재초기화는 지원하지 않는다.
	 */
	virtual void Shutdown() = 0;

	/**
	 * 단일 창의 그래픽스 출력 자원을 새 픽셀 크기에 맞춘다. OS 창의 크기는 바꾸지 않는다.
	 * 초기화 성공 후 프레임 출력 전에 호출한다. 최소화 상태이거나 크기가 0이면 호출하지 않는다.
	 * 출력 자원 재구성 실패는 구현이 치명 종료로 처리한다.
	 * @param SizeX 0보다 큰 클라이언트 영역의 가로 픽셀 수.
	 * @param SizeY 0보다 큰 클라이언트 영역의 세로 픽셀 수.
	 */
	virtual void Resize(uint32 SizeX, uint32 SizeY) = 0;

	/**
	 * 단일 창에 한 프레임을 그려 화면에 제출한다.
	 * 초기화 성공 후 필요한 크기 변경을 적용하고 호출한다.
	 * 최소화 상태이거나 출력 크기가 0이면 호출하지 않는다.
	 * 화면 제출 실패는 구현이 치명 종료로 처리한다.
	 * renderData는 호출 중에만 빌려 읽으며 저장하지 않는다. 호출이 끝날 때까지 유효하고 변경되지 않아야 한다.
	 * 현재 메시 데이터는 사용하지 않으며 빈 목록도 배경색 출력과 화면 제출을 수행한다.
	 */
	virtual void RenderFrame(const TArray<FMeshRenderData>& renderData) = 0;
};
