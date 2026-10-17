cbuffer TransformBuffer : register(b0)
{
    row_major float4x4 LocalToClip;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VertexOutput Main(float3 position : POSITION, float2 uv : TEXCOORD0)
{
    VertexOutput output;
    output.position = mul(float4(position, 1.0f), LocalToClip);
    output.uv = uv;
    return output;
}
