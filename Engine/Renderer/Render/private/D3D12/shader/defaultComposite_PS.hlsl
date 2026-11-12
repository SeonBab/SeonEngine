#define REGISTERS_NO_BUILTIN_OBJECT
#include "common/registers.hlsli"

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};


float4 PSMain(PSInput input) : SV_TARGET
{
    const uint layerCount = (uint)g_RootConstants.x;

    float4 color = g_SR0.Sample(g_SamplerLinearClamp, input.UV);
    float4 next;

    if (layerCount > 1) { next = g_SR1.Sample(g_SamplerLinearClamp, input.UV); color.rgb = lerp(color.rgb, next.rgb, next.a); }
    if (layerCount > 2) { next = g_SR2.Sample(g_SamplerLinearClamp, input.UV); color.rgb = lerp(color.rgb, next.rgb, next.a); }
    if (layerCount > 3) { next = g_SR3.Sample(g_SamplerLinearClamp, input.UV); color.rgb = lerp(color.rgb, next.rgb, next.a); }
    if (layerCount > 4) { next = g_SR4.Sample(g_SamplerLinearClamp, input.UV); color.rgb = lerp(color.rgb, next.rgb, next.a); }
    if (layerCount > 5) { next = g_SR5.Sample(g_SamplerLinearClamp, input.UV); color.rgb = lerp(color.rgb, next.rgb, next.a); }
    if (layerCount > 6) { next = g_SR6.Sample(g_SamplerLinearClamp, input.UV); color.rgb = lerp(color.rgb, next.rgb, next.a); }
    if (layerCount > 7) { next = g_SR7.Sample(g_SamplerLinearClamp, input.UV); color.rgb = lerp(color.rgb, next.rgb, next.a); }

    return float4(color.rgb, 1.0f);
}
