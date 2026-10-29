// 텍스처와 정점 색상을 곱하고 정점 셰이더가 계산한 정반사를 더한다.
// 마스킹 재질은 낮은 알파의 픽셀을 제거해 색과 깊이를 기록하지 않는다.
// VS 출력과 필드 순서를 맞춰 같은 입력 레지스터에서 색상과 UV를 읽는다.
struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	float3 specular : COLOR1;
};

Texture2D BaseTexture : register(t0);
SamplerState BaseSampler : register(s0);

// CPU의 FAlphaMaskConstants와 같은 16바이트 배치다. UI의 PS b0와 구분한다.
cbuffer AlphaMaskConstantBuffer : register(b1)
{
	uint bAlphaMasked;
	float AlphaCutoff;
	uint2 AlphaMaskPadding;
};

float4 Main(PSInput input) : SV_Target
{
	float4 baseColor = BaseTexture.Sample(BaseSampler, input.uv) * input.color;

	if (bAlphaMasked != 0)
	{
		clip(baseColor.a - AlphaCutoff);
	}

	return float4(baseColor.rgb + input.specular, baseColor.a);
}
