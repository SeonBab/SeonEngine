// CPU의 행 우선 행렬을 전치 없이 받는다. 현재 출력 영역의 픽셀 위치를 클립 좌표로 변환한다.
cbuffer UITransformConstantBuffer : register(b0)
{
	row_major float4x4 PixelToClip;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};

VSOutput Main(float2 position : POSITION, float2 uv : TEXCOORD0, float4 color : COLOR0)
{
	VSOutput output;
	output.position = mul(float4(position, 0.0f, 1.0f), PixelToClip);
	output.color = color;
	output.uv = uv;
	return output;
}
