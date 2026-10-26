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

// SkyLight의 세기 반영 RGB를 모든 방향에 동일한 주변광으로 적용한다.
cbuffer SkyLightConstantBuffer : register(b2)
{
	float3 AmbientLightColor;
	float AmbientLightPadding;
};

// CPU의 FSpecularConstants와 같은 112바이트 배치다. 현재 단계에서는 선언만 하고 Main에서 읽지 않는다.
cbuffer SpecularConstantBuffer : register(b3)
{
	row_major float4x4 LocalToWorld;
	float3 ViewLocation;
	float ViewLocationPadding;
	float3 OrthographicViewDirection;
	uint IsOrthographic;
	float3 SpecularColor;
	float Shininess;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float3 specular : COLOR1;
};

VSOutput Main(float3 position : POSITION, float4 color : COLOR0, float3 normal : NORMAL0, float2 uv : TEXCOORD0)
{
	float3 worldPosition = mul(float4(position, 1.0f), LocalToWorld).xyz;
	float3 worldNormal = mul(normal, (float3x3)NormalToWorld);
	float3 N = normalize(worldNormal);
	float3 L = -LightDirection;
	float3 toCamera = IsOrthographic != 0 ? OrthographicViewDirection : ViewLocation - worldPosition;
	float3 V = float3(0.0f, 0.0f, 0.0f);

	if (dot(toCamera, toCamera) > 1.0e-8f)
	{
		V = normalize(toCamera);
	}

	float3 halfVector = L + V;
	float3 H = float3(0.0f, 0.0f, 0.0f);

	if (dot(V, V) > 0.0f && dot(halfVector, halfVector) > 1.0e-8f)
	{
		H = normalize(halfVector);
	}

	float NdotH = saturate(dot(N, H));

	float3 specular = float3(0.0f, 0.0f, 0.0f);

	if (dot(N, L) > 0.0f)
	{
		float specularFactor = pow(NdotH, Shininess);
		specular = LightColor * SpecularColor * specularFactor;
	}

	float diffuseFactor = max(dot(N, L), 0.0f);
	float3 litColor = color.rgb * (AmbientLightColor + LightColor * diffuseFactor);

	VSOutput output;
	output.position = mul(float4(position, 1.0f), LocalToClip);
	output.color = float4(litColor, color.a);
	output.normal = worldNormal;
	output.uv = uv;
	output.specular = specular;

	return output;
}
