// CPU에서 결합한 행 우선 로컬→클립 행렬을 정점에 적용한다.
cbuffer TransformConstantBuffer : register(b0)
{
	row_major float4x4 LocalToClip;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

VSOutput Main(float3 Position : POSITION, float4 Color : COLOR0)
{
	VSOutput output;
	// 클립 좌표를 출력한다. 클리핑과 원근 나누기는 이후 GPU 단계에서 수행한다.
	output.Position = mul(float4(Position, 1.0f), LocalToClip);
	output.Color = Color;
	return output;
}
