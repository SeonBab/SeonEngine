#include "StaticMesh.h"

#include <utility>

StaticMesh::StaticMesh(std::string newName, std::string newPath)
	: Asset(std::move(newName), std::move(newPath))
{
}
