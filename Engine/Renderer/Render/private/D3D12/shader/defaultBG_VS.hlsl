struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

PSInput VSMain(uint vertexID : SV_VertexID)
{
    PSInput result;

    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);

    result.Pos = float4(uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 1.0f, 1.0f);
    result.UV = uv;

    return result;
}
