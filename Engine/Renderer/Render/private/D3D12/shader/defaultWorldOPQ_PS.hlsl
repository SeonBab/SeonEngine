#if defined(OUTPUT_COLORID) || defined(OUTPUT_DEPTHONLY)
#error "defaultWorldOPQ_PS: Color / GBuffer 레이아웃만 지원"
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


void SampleSurface(PSInput input, out float4 albedo, out float3 worldNormal, out float rough, out float metal, out float ao, out float3 emission)
{
    const float4 texColor = g_Tex[0].Sample(g_SamplerAnisoWrap, input.uv);
    const float4 normalVal = g_Tex[1].Sample(g_SamplerAnisoWrap, input.uv);
    const float4 orm = g_Tex[2].Sample(g_SamplerAnisoWrap, input.uv);
    emission = g_Tex[3].Sample(g_SamplerAnisoWrap, input.uv).rgb;

    albedo = texColor * obj_Col;

    const float3 T = normalize(input.tangent);
    const float3 N = normalize(input.normal);
    const float3 B = cross(N, T);
    const float3x3 TBN = float3x3(T, B, N);
    const float3 tangentNormal = normalVal.xyz * 2.0f - 1.0f;
    worldNormal = normalize(mul(tangentNormal, TBN));

    ao = orm.r;
    rough = orm.g * obj_Rough;
    metal = orm.b * obj_Metal;
}


#if defined(OUTPUT_GBUFFER)

struct PSOutput
{
    float4 albedo   : SV_TARGET0;
    float4 normal   : SV_TARGET1;
    float4 orm      : SV_TARGET2;
    float4 emission : SV_TARGET3;
};

PSOutput PSMain(PSInput input)
{
    float4 albedo; float3 worldNormal; float rough; float metal; float ao; float3 emission;
    SampleSurface(input, albedo, worldNormal, rough, metal, ao, emission);

    PSOutput o;
    o.albedo   = float4(albedo.rgb, 0.0f);
    o.normal   = float4(worldNormal * 0.5f + 0.5f, 0.0f);
    o.orm      = float4(ao, saturate(rough), saturate(metal), 0.0f);
    o.emission = float4(emission, 0.0f);
    return o;
}

#else

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 albedo; float3 worldNormal; float rough; float metal; float ao; float3 emission;
    SampleSurface(input, albedo, worldNormal, rough, metal, ao, emission);

    const float3 V = normalize(g_EyePos - input.worldPos);
    const float3 color = ShadeSurface(input.worldPos, worldNormal, V, albedo.rgb, rough, metal, ao) + emission;

    return float4(color, albedo.a);
}

#endif
