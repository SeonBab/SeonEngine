// PS b0는 VS b0와 독립적이다. CPU는 패딩을 포함한 16바이트 전체를 전달한다.
cbuffer UITextureConstantBuffer : register(b0)
{
	// 0은 Straight, 그 외는 Premultiplied다. 실제 텍스처의 알파 표현에 맞춰 지정한다.
	uint SourcePremultiplied;
	uint3 Padding;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};

Texture2D BaseTexture : register(t0);
SamplerState BaseSampler : register(s0);

// 위젯 색상은 Straight RGBA다. 출력은 원본의 알파 표현을 유지하므로 백엔드가 그에 맞는 블렌드를 선택해야 한다.
float4 Main(PSInput input) : SV_Target
{
	float4 result = BaseTexture.Sample(BaseSampler, input.uv) * input.color;
	if (SourcePremultiplied != 0)
	{
		// 원본 RGB에는 텍스처 알파가 이미 반영되어 있으므로 위젯 알파만 추가로 곱한다.
		result.rgb *= input.color.a;
	}

	return result;
}
