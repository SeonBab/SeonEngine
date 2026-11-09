#pragma once

#include "Input/InputDispatcher.h"
#include "Renderer/RenderScene.h"

#include <memory>
#include <vector>

class CameraComponent;
class GameObject;

/** 메인 스레드에서 사용하며 등록한 게임 객체를 단독 소유한다. */
class World
{
	// 생성과 소멸
public:
	World();
	~World();

	World(const World&) = delete;
	World& operator=(const World&) = delete;
	World(World&&) = delete;
	World& operator=(World&&) = delete;

	// 입력
public:
	/** 이 월드의 입력 전달자를 빌려준다. 월드 수명 동안 유효하다. */
	InputDispatcher& GetInputDispatcher();

private:
	// gameObjects보다 먼저 선언해 더 늦게 파괴한다. 소멸하는 InputComponent가 전달자에서 자신을 제거할 수 있게 하기 위해서다.
	InputDispatcher inputDispatcher;

	// 렌더
public:
	/** 이 월드의 렌더 씬을 빌려준다. 월드 수명 동안 유효하다. */
	RenderScene& GetRenderScene();

private:
	// gameObjects보다 먼저 선언해 더 늦게 파괴한다. 이후 소멸하는 렌더 컴포넌트가 자신을 해제할 수 있게 하기 위해서다.
	RenderScene renderScene;

	// 객체 관리
public:
	/**
	 * 소유권을 받아 등록하고 월드 수명 동안 유효한 비소유 포인터를 반환한다.
	 * 빈 입력과 소속 불일치는 nullptr를 반환하고 입력 객체는 정리한다.
	 * 갱신 중에는 호출하지 않는다. 할당 실패 예외는 전파한다.
	 */
	GameObject* AddGameObject(std::unique_ptr<GameObject> newGameObject);

	/** 이 World에 등록된 객체인지 확인한다. nullptr는 false다. */
	bool ContainsGameObject(const GameObject* gameObject) const;

private:
	std::vector<std::unique_ptr<GameObject>> gameObjects;

	// 카메라
public:
	/**
	 * 이 월드에 등록된 객체에 붙은 카메라를 활성으로 지정한다. nullptr는 해제다.
	 * 부착 전이거나 다른 월드, 미등록 객체의 카메라면 기존 값을 유지하고 false를 반환한다.
	 */
	bool SetActiveCamera(CameraComponent* camera);

	/** 활성 카메라를 빌려준다. 지정하지 않았으면 nullptr이다. */
	CameraComponent* GetActiveCamera() const { return activeCamera; }

private:
	// 비소유. 이 월드가 소유한 객체의 컴포넌트를 가리킨다. 객체 제거 기능이 생기면 해제 시점을 함께 정한다.
	CameraComponent* activeCamera = nullptr;

	// 갱신
public:
	/**
	 * 등록 순서대로 갱신한다. deltaTime는 초 단위다.
	 * 콜백에서 객체 등록과 같은 월드의 Tick 재호출을 하지 않는다.
	 */
	void Tick(float deltaTime);
};
