#pragma once

#include "AssetManager.h"
#include "WindowsApplication.h"
#include "World.h"

#include <memory>

namespace Hakko
{
	class D3D12Renderer;
}

/**
 * 창, 렌더러, AssetManager 및 World를 소유하고 프레임 순서(메시지 처리, 입력, 갱신, 렌더링)를 실행한다.
 * 메인 스레드에서 생성, 초기화, 실행, 소멸한다. 게임은 Run 전에 GetWorld로 World를 꾸민다.
 */
class Engine
{
	// 생성과 소멸
public:
	Engine();
	~Engine();

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(Engine&&) = delete;

	/** 창을 만든다. 실패하거나 이미 초기화했으면 false를 반환한다. */
	bool Initialize();

	/**
	 * GPU 작업을 마치고 메시 등록 캐시, DataHub, 렌더러와 창을 순서대로 정리한다.
	 * 실행 루프 밖에서 호출하며 반복 호출할 수 있다. 창 정리 실패 시 false다.
	 */
	bool Shutdown();

	// 실행
public:
	/** 초기화 성공 후 Run 전에 게임이 구성할 World를 빌려준다. 엔진 수명 동안 유효하다. */
	World& GetWorld();

	/**
	 * 렌더링을 초기화하고 창이 닫힐 때까지 프레임을 반복한 뒤 종료 코드를 반환한다.
	 * Initialize 성공 후 한 번만 호출한다. 실행할 수 없거나 초기 창 크기가 유효하지 않으면 1을 반환한다.
	 * DataHub 또는 렌더러가 초기화 실패를 반환하거나 창 정리에 실패해도 1을 반환한다.
	 * 실행 중 창 크기 조회나 메시 준비 또는 그리기 요청 준비 실패 시 Shutdown 후 1을 반환한다.
	 * 최소화 또는 크기가 0 이하면 메시 준비와 그리기를 건너뛴다.
	 * 활성 카메라가 없거나 시점 조회 또는 카메라 준비에 실패해도 해당 프레임의 메시 준비와 그리기를 건너뛴다.
	 * 렌더링 초기화 실패와 정상 종료에서도 Shutdown을 호출한다.
	 */
	int Run();

private:
	/** DataHub와 렌더러를 준비한다. 성공한 렌더러만 멤버에 저장하고, 실패하면 DataHub를 정리한다. */
	bool InitializeRendering();

	/** 한 프레임을 처리한다. deltaTime는 초 단위다. */
	void Tick(float deltaTime);

private:
	// Shutdown에서 렌더러를 먼저 해제한다. 멤버 자동 소멸에서도 창 객체가 가장 오래 살아 있도록 둔다.
	WindowsApplication windowsApplication;
	// World가 정리될 때까지 매니저가 살아 있도록 World보다 먼저 선언한다.
	AssetManager assetManager;
	World world;
	// 초기화에 성공한 렌더러만 소유한다. nullptr이면 Finalize할 렌더러가 없다.
	std::unique_ptr<Hakko::D3D12Renderer> renderer;
	// 초기화 또는 OnResize에 마지막으로 전달한 클라이언트 영역의 픽셀 크기다.
	int renderWidth = 0;
	int renderHeight = 0;

	// Initialize 성공 시 true, Run을 시작하면 false다. 창 없이 실행하거나 종료 후 다시 실행하는 것을 막는다.
	bool canRun = false;
};
