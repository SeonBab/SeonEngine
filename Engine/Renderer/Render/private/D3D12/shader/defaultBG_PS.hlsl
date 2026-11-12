#if defined(OUTPUT_GBUFFER) || defined(OUTPUT_COLORID) || defined(OUTPUT_DEPTHONLY)
#error "defaultBG_PS: Color 레이아웃만 지원"
#endif
#define REGISTERS_NO_BUILTIN_OBJECT
#include "common/registers.hlsli"

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};


float4 PSMain(PSInput input) : SV_TARGET
{
    float3 color = g_Tex[0].Sample(g_SamplerLinearClamp, input.UV).rgb;

    return float4(color * g_RootConstants.rgb, 1.0f);
}
