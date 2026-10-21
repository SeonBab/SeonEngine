// CPU 행렬의 행 우선 저장과 맞춰 전치 없이 전달받는다.
cbuffer TransformConstantBuffer : register(b0)
{
	row_major float4x4 LocalToClip;
	row_major float4x4 NormalToWorld;
};

cbuffer DirectionalLightConstantBuffer : register(b1)
{
	float3 LightDirection;
	float3 LightColor;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

VSOutput Main(float3 position : POSITION, float4 color : COLOR0, float3 normal : NORMAL0, float2 uv : TEXCOORD0)
{
	float3 worldNormal = mul(normal, (float3x3)NormalToWorld);
	float3 N = normalize(worldNormal);
	float3 L = -LightDirection;

	float diffuseFactor = max(dot(N, L), 0.0f);
	float3 litColor = color.rgb * LightColor * diffuseFactor;

	VSOutput output;
	output.position = mul(float4(position, 1.0f), LocalToClip);
	output.color = float4(litColor, color.a);
	output.normal = worldNormal;
	output.uv = uv;

	return output;
}
