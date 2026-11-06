#pragma once

#include "Component/Component.h"

#include <concepts>
#include <memory>
#include <vector>

class World;

class GameObject
{
	// 생성과 소멸
public:
	/** 월드는 이 객체보다 오래 살아 있어야 한다. 생성과 등록은 별개다. */
	explicit GameObject(World& targetWorld);
	virtual ~GameObject();

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = delete;
	GameObject& operator=(GameObject&&) = delete;

	// 소속 월드
public:
	World& GetWorld() const;

private:
	World& world;

	// 컴포넌트
public:
	/**
	 * 소유권을 받아 등록하고 이 객체를 소속으로 설정한 뒤 컴포넌트의 OnRegister를 호출한다.
	 * 반환 포인터는 이 객체의 수명 동안만 빌려 쓴다.
	 * 빈 입력은 nullptr를 반환한다.
	 */
	Component* AddComponent(std::unique_ptr<Component> newComponent);

	/**
	 * 붙은 컴포넌트를 추가 순서대로 확인해 T 타입인 첫 번째를 빌려준다. 없으면 nullptr이다.
	 * 반환 포인터는 이 객체의 수명 동안만 유효하다.
	 */
	template <typename T>
		requires std::derived_from<T, Component>
	T* GetComponent() const;

private:
	std::vector<std::unique_ptr<Component>> components;

	// 갱신
public:
	/** 월드가 프레임마다 호출한다. deltaTime는 초 단위다. */
	void Tick(float deltaTime);

protected:
	/** 파생 타입의 갱신 지점. 부모 호출은 필요 없다. */
	virtual void OnTick(float deltaTime);
};

template <typename T>
	requires std::derived_from<T, Component>
T* GameObject::GetComponent() const
{
	for (const std::unique_ptr<Component>& component : components)
	{
		if (T* found = dynamic_cast<T*>(component.get()))
		{
			return found;
		}
	}
	return nullptr;
}
