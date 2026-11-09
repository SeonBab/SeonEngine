#include "PlayerObject.h"

#include "Component/CameraComponent.h"
#include "Component/InputComponent.h"
#include "Component/TransformComponent.h"
#include "Input/Key.h"
#include "Input/KeyBinding.h"

#include <cstdio>
#include <memory>
#include <utility>

namespace
{
	// 단위는 cm/s다. 1 unit = 1cm 기준으로 초당 3m 이동한다.
	constexpr float MoveSpeed = 300.0f;
}

PlayerObject::PlayerObject(World& targetWorld)
	: GameObject(targetWorld)
{
	transform = GetComponent<TransformComponent>();

	std::unique_ptr<InputComponent> newInput = std::make_unique<InputComponent>();
	InputComponent* input = newInput.get();
	// 부착 시 OnRegister가 호출되어 소속 World의 전달자에 추가된다.
	AddComponent(std::move(newInput));

	// 왼손 좌표계에서 +Z는 앞쪽, +X는 오른쪽이다.
	input->BindKey(Key::W, KeyTrigger::Held, [this](float deltaTime) { Move(Vector3{ 0.0f, 0.0f, 1.0f }, deltaTime); });
	input->BindKey(Key::S, KeyTrigger::Held, [this](float deltaTime) { Move(Vector3{ 0.0f, 0.0f, -1.0f }, deltaTime); });
	input->BindKey(Key::A, KeyTrigger::Held, [this](float deltaTime) { Move(Vector3{ -1.0f, 0.0f, 0.0f }, deltaTime); });
	input->BindKey(Key::D, KeyTrigger::Held, [this](float deltaTime) { Move(Vector3{ 1.0f, 0.0f, 0.0f }, deltaTime); });
	input->BindKey(Key::Space, KeyTrigger::Pressed, [this](float) { PrintPosition("Space Pressed"); });
	input->BindKey(Key::Space, KeyTrigger::Released, [this](float) { PrintPosition("Space Released"); });

	// 플레이어 위치와 회전을 그대로 쓰는 1인칭 시점 카메라다. 활성 지정은 World 등록 뒤 GameMain이 한다.
	AddComponent(std::make_unique<CameraComponent>());
}

void PlayerObject::Move(const Vector3& direction, float deltaTime)
{
	const float distance = MoveSpeed * deltaTime;

	Vector3 position = transform->GetPosition();
	position.x += direction.x * distance;
	position.y += direction.y * distance;
	position.z += direction.z * distance;
	transform->SetPosition(position);
}

void PlayerObject::PrintPosition(const char* label) const
{
	const Vector3 position = transform->GetPosition();
	std::printf("%s: (%.1f, %.1f, %.1f) cm\n", label, position.x, position.y, position.z);
}
