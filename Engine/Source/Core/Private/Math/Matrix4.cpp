#include "Math/Matrix4.h"

void FMatrix4::SetIdentity()
{
	*this = FMatrix4{};
	m[0][0] = 1.0f;
	m[1][1] = 1.0f;
	m[2][2] = 1.0f;
	m[3][3] = 1.0f;
}
