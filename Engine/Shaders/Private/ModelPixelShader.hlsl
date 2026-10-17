Texture2D BaseTexture : register(t0);
SamplerState BaseSampler : register(s0);

float4 Main(float4 position : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
{
    return BaseTexture.Sample(BaseSampler, uv);
}
