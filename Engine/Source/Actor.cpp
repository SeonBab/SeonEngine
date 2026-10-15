#include "Actor.h"

#include "ActorComponent.h"
#include "TransformComponent.h"

#include <typeinfo>
#include <utility>

AActor::AActor()
{
	components.push_back(MakeUnique<FTransformComponent>(*this));
}

AActor::~AActor() = default;

bool AActor::AddComponent(TUniquePtr<FActorComponent>& component)
{
	if (!component || &component->GetOwner() != this)
	{
		return false;
	}

	if (!component->AllowsMultipleComponents())
	{
		const std::type_info& componentType = typeid(*component);

		for (const auto& existingComponent : components)
		{
			if (typeid(*existingComponent) == componentType)
			{
				return false;
			}
		}
	}

	components.push_back(std::move(component));

	return true;
}
