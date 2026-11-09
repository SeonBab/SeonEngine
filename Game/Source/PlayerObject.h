#pragma once

#include "Core/Math/Vector3.h"
#include "GameObject.h"

class TransformComponent;
class World;

/** 입력 확인용 게임 객체다. WASD로 수평 이동하고 Space의 누름과 뗌을 콘솔에 출력한다. 1인칭 카메라를 가진다. */
class PlayerObject : public GameObject
{
	// 생성과 소멸
public:
	/** 기본 TransformComponent를 조회하고 입력, 카메라 컴포넌트를 부착하며 키 바인딩을 등록한다. */
	explicit PlayerObject(World& targetWorld);

	// 이동
private:
	/** direction은 월드 축 방향 단위 벡터다. deltaTime은 초 단위다. */
	void Move(const Vector3& direction, float deltaTime);

	/** label과 현재 위치를 cm 단위로 콘솔에 한 줄 출력한다. */
	void PrintPosition(const char* label) const;

private:
	// 비소유. GameObject 생성자가 부착한 TransformComponent를 가리킨다.
	TransformComponent* transform = nullptr;
};
