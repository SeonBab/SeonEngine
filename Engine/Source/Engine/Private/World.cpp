#include "World.h"

#include "Actor.h"

FWorld::FWorld() = default;
FWorld::~FWorld() = default;

FActor& FWorld::SpawnActor()
{
	actors.push_back(MakeUnique<FActor>());
	return *actors.back();
}
