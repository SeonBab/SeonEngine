#if defined(OUTPUT_GBUFFER) || defined(OUTPUT_COLORID) || defined(OUTPUT_DEPTHONLY)
#error "defaultWorldTP_PS: Color 레이아웃만 지원"
#endif


#include "common/lighting.hlsli"


struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 worldPos : TEXCOORD3;
};


float4 PSMain(PSInput input) : SV_TARGET
{
    const float4 texColor = g_Tex[0].Sample(g_SamplerAnisoWrap, input.uv);
    const float4 normalVal = g_Tex[1].Sample(g_SamplerAnisoWrap, input.uv);
    const float4 orm = g_Tex[2].Sample(g_SamplerAnisoWrap, input.uv);
    const float3 emission = g_Tex[3].Sample(g_SamplerAnisoWrap, input.uv).rgb;

    const float4 albedo = texColor * obj_Col;

    const float3 T = normalize(input.tangent);
    const float3 N = normalize(input.normal);
    const float3 B = cross(N, T);
    const float3x3 TBN = float3x3(T, B, N);
    const float3 tangentNormal = normalVal.xyz * 2.0f - 1.0f;
    const float3 worldNormal = normalize(mul(tangentNormal, TBN));

    const float3 V = normalize(g_EyePos - input.worldPos);
    const float3 color = ShadeSurface(input.worldPos, worldNormal, V, albedo.rgb, orm.g * obj_Rough, orm.b * obj_Metal, orm.r) + emission;

    return float4(color, albedo.a);
}
