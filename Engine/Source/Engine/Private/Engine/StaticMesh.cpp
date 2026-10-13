#include "Engine/StaticMesh.h"

UStaticMesh::UStaticMesh(UObject* inOuter)
	: UObject(inOuter)
{
}

bool UStaticMesh::SetMeshData(const TArray<FStaticMeshVertex>& inVertices, EPrimitiveType inPrimitiveType)
{
	switch (inPrimitiveType)
	{
		case EPrimitiveType::PT_TriangleList:
			if (inVertices.size() % 3 != 0)
			{
				return false;
			}
			break;
		case EPrimitiveType::PT_TriangleStrip:
			if (!inVertices.empty() && inVertices.size() < 3)
			{
				return false;
			}
			break;
		default:
			return false;
	}

	vertices = inVertices;
	primitiveType = inPrimitiveType;
	return true;
}

const TArray<FStaticMeshVertex>& UStaticMesh::GetVertices() const
{
	return vertices;
}

EPrimitiveType UStaticMesh::GetPrimitiveType() const
{
	return primitiveType;
}
