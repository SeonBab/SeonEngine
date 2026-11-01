// 두 텍스처의 선택 연산 뒤 정점 색상/조명을 곱하고 기존 정반사를 더한다.
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
Texture2D BlendTexture : register(t1);
SamplerState BlendSampler : register(s1);

// CPU의 FAlphaMaskConstants와 같은 16바이트 배치다. UI의 PS b0와 구분한다.
cbuffer AlphaMaskConstantBuffer : register(b1)
{
	uint bAlphaMasked;
	float AlphaCutoff;
	uint2 AlphaMaskPadding;
};

// CPU의 FTextureBlendConstants와 같은 16바이트 배치다.
cbuffer TextureBlendConstantBuffer : register(b2)
{
	float BlendFactor;
	uint TextureBlendMode;
	float2 TextureBlendPadding;
};

float4 Main(PSInput input)
	: SV_Target
{
	float4 firstColor = BaseTexture.Sample(BaseSampler, input.uv);
	float4 secondColor = BlendTexture.Sample(BlendSampler, input.uv);
	// TextureBlendMode.h: 0=Lerp, 1=AddSmooth. AddSmooth는 마스크 알파로 실루엣을 바꾸지 않는다.
	float4 textureColor;
	if (TextureBlendMode == 1)
	{
		float3 maskColor = secondColor.rgb * BlendFactor;
		textureColor = float4(firstColor.rgb + maskColor - firstColor.rgb * maskColor, firstColor.a);
	}
	else
	{
		textureColor = lerp(firstColor, secondColor, BlendFactor);
	}
	float4 baseColor = textureColor * input.color;

	if (bAlphaMasked != 0)
	{
		clip(baseColor.a - AlphaCutoff);
	}

	return float4(baseColor.rgb + input.specular, baseColor.a);
}
