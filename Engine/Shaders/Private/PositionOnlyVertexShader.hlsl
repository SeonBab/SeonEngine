// CPU 행 우선 행렬을 그대로 읽어 로컬 정점에 적용한다. 카메라 변환은 아직 없다.
cbuffer TransformConstantBuffer : register(b0)
{
	row_major float4x4 LocalToWorld;
};

float4 Main(float3 Position : POSITION) : SV_POSITION
{
	// 현재는 월드 위치를 클립 공간 출력으로 직접 사용한다.
	return mul(float4(Position, 1.0f), LocalToWorld);
}
