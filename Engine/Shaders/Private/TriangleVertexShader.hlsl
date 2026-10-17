// CPU 행렬의 행 우선 저장과 맞춰 전치 없이 전달받는다.
cbuffer TransformConstantBuffer : register(b0)
{
	row_major float4x4 LocalToClip;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};

VSOutput Main(float3 position : POSITION, float4 color : COLOR0, float2 uv : TEXCOORD0)
{
	VSOutput output;
	output.position = mul(float4(position, 1.0f), LocalToClip);
	output.color = color;
	output.uv = uv;
	return output;
}
