// VS 출력과 필드 순서를 맞춰 같은 입력 레지스터에서 색상과 UV를 읽는다.
struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};

Texture2D BaseTexture : register(t0);
SamplerState BaseSampler : register(s0);

float4 Main(PSInput input)
	: SV_Target
{
	return BaseTexture.Sample(BaseSampler, input.uv) * input.color;
}
