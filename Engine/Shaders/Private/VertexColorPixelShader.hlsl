// VS 출력과 같은 순서로 위치 / 색상 / UV 입력을 선언해 단계 간 시그니처를 맞춘다.
struct PSInput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
	float2 UV : TEXCOORD0;
};

// UV 입력은 후속 텍스처 샘플링을 위한 계약이며 현재 출력에는 사용하지 않는다.
// 정점 사이에서 보간된 선형 RGBA 색상을 출력한다. 텍스처 / 조명 / 블렌딩은 추가하지 않는다.
float4 Main(PSInput input) : SV_Target
{
	return input.Color;
}
