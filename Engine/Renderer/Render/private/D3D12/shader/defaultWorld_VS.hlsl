#include "common/registers.hlsli"


#if VERTEX_SKINNED
cbuffer BoneCB : OBJECT_CB(1)
{
    matrix obj_BoneTr[SKIN_BONE_MAX];
};
#endif


struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 worldPos : TEXCOORD3;
};


#if VERTEX_SKINNED
PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD,
                float3 normal : NORMAL, float3 tangent : TANGENT,
                uint4 boneIdx : BLENDINDICES, float4 boneWeight : BLENDWEIGHT)
#else
PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD,
                float3 normal : NORMAL, float3 tangent : TANGENT)
#endif
{
    PSInput result;

    float3 posL = position.xyz;
    float3 nrmL = normal;
    float3 tanL = tangent;

#if VERTEX_SKINNED
    const float wSum = boneWeight.x + boneWeight.y + boneWeight.z + boneWeight.w;
    if (wSum > 0.0f)
    {
        matrix skin = boneWeight.x * obj_BoneTr[boneIdx.x]
                    + boneWeight.y * obj_BoneTr[boneIdx.y]
                    + boneWeight.z * obj_BoneTr[boneIdx.z]
                    + boneWeight.w * obj_BoneTr[boneIdx.w];

        posL = mul(float4(posL, 1.0f), skin).xyz;
        nrmL = mul(nrmL, (float3x3)skin);
        tanL = mul(tanL, (float3x3)skin);
    }
#endif

    float4 posW = mul(float4(posL, 1.0f), obj_W);
    result.position = mul(posW, g_VP);
    result.worldPos = posW.xyz;

    result.uv = uv;
    result.normal = mul(nrmL, (float3x3)obj_W);
    result.tangent = mul(tanL, (float3x3)obj_W);

    return result;
}
