// 초기 도형 출력은 텍스처 / 조명 없이 고정 흰색을 사용해 마젠타 배경과 구분한다.
float4 Main() : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
