#if defined(OUTPUT_GBUFFER) || defined(OUTPUT_COLORID) || defined(OUTPUT_DEPTHONLY)
#error "defaultSkyBox_PS: Color 레이아웃만 지원"
#endif

cbuffer RootConstantsCB : register(b4, space0)
{
    float4 g_RootConstants;
};

TextureCube  g_SkyboxTex         : register(t0, space1);
SamplerState g_SamplerLinearClamp : register(s1);

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 RayDir : TEXCOORD0;
};


float4 PSMain(PSInput input) : SV_TARGET
{
    float3 color = g_SkyboxTex.SampleLevel(g_SamplerLinearClamp, normalize(input.RayDir), 0).rgb;

    return float4(color * g_RootConstants.rgb, 1.0f);
}
