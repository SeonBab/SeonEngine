#include "Actor.h"
#include "GameProject.h"
#include "Math/Vector3.h"
#include "StaticMeshComponent.h"
#include "World.h"

namespace
{
	// 구체 게임 구현은 이 파일 안에 두고 엔진에는 공통 인터페이스만 제공한다.
	class FSampleGameProject : public IGameProject
	{
	public:
		/** 화면 출력 확인을 위한 삼각형 액터를 배치하고 메시 추가 성공 여부를 반환한다. */
		bool InitializeWorld(FWorld& world) override
		{
			FActor& actor = world.SpawnActor();
			TUniquePtr<FStaticMeshComponent> mesh = MakeUnique<FStaticMeshComponent>(actor);

			// 현재 셰이더는 변환 행렬 없이 정점을 클립 공간 위치로 사용한다.
			mesh->SetVertices({
				FVector3{-0.5f, -0.5f, 0.0f},
				FVector3{0.0f, 0.5f, 0.0f},
				FVector3{0.5f, -0.5f, 0.0f},
			});

			return actor.AddComponent(mesh);
		}
	};
}

TUniquePtr<IGameProject> CreateGameProject()
{
	return MakeUnique<FSampleGameProject>();
}
