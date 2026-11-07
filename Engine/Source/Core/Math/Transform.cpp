#include "Transform.h"

#include "Matrix4x4.h"
#include "Quaternion.h"

Matrix4x4 Transform::ToMatrix() const
{
	return Matrix4x4::CreateScale(scale) * Matrix4x4::CreateFromQuaternion(rotation) * Matrix4x4::CreateTranslation(position);
}
