// CPU 행 우선 행렬을 그대로 읽어 로컬 정점에 적용한다. 카메라 변환은 아직 없다.
cbuffer TransformConstantBuffer : register(b0)
{
	row_major float4x4 LocalToWorld;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

VSOutput Main(float3 Position : POSITION, float4 Color : COLOR0)
{
	VSOutput output;
	// 현재는 월드 위치를 클립 공간 출력으로 직접 사용한다.
	output.Position = mul(float4(Position, 1.0f), LocalToWorld);
	output.Color = Color;
	return output;
}
