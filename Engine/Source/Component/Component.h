#pragma once

class GameObject;

class Component
{
	// 생성과 소멸
public:
	/** 소속 객체가 없는 상태로 생성한다. 부착은 GameObject::AddComponent가 담당한다. */
	Component();
	virtual ~Component();

	Component(const Component&) = delete;
	Component& operator=(const Component&) = delete;
	Component(Component&&) = delete;
	Component& operator=(Component&&) = delete;

	// 소속 객체
public:
	/** 소속 설정 전에는 nullptr를 반환한다. 반환 포인터는 소유권을 갖지 않는다. */
	GameObject* GetGameObject() const;

	/**
	 * 소속 포인터만 설정하며 소유권을 이전하거나 부착하지 않는다.
	 * 대상 객체는 이 컴포넌트보다 오래 살아 있어야 한다.
	 * 부착 후에는 실제 소유 객체와 다른 소속을 설정하지 않는다.
	 */
	void SetGameObject(GameObject* targetGameObject);

	/**
	 * GameObject::AddComponent가 소속을 설정한 직후 한 번 호출한다.
	 * 파생 타입은 소속 World에 필요한 등록을 여기서 한다. 기본 구현은 비어 있다.
	 */
	virtual void OnRegister();

private:
	GameObject* gameObject = nullptr;

	// 갱신
public:
	/** 프레임 경과 시간은 초 단위다. */
	virtual void Tick(float deltaTime);
};
