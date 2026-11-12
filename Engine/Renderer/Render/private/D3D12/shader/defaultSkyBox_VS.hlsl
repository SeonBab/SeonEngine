#include "common/registers.hlsli"

struct PSInput
{
    float4 Pos : SV_POSITION;
    float3 RayDir : TEXCOORD0;
};


PSInput VSMain(uint vertexID : SV_VertexID)
{
    PSInput result;

    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    float4 ndc = float4(uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 1.0f, 1.0f);

    result.Pos = ndc;

    float3 viewDir = float3(ndc.x / g_P._11, ndc.y / g_P._22, 1.0f);

    float3x3 view3x3 = (float3x3)g_V;
    float3x3 invView3x3 = transpose(view3x3);

    result.RayDir = mul(viewDir, invView3x3);

    return result;
}
