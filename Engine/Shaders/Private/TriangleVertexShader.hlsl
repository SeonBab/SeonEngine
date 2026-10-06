// 첫 삼각형은 입력 위치를 클립 공간 좌표로 사용한다. 월드 / 카메라 변환은 아직 적용하지 않는다.
float4 Main(float3 Position : POSITION) : SV_POSITION
{
	return float4(Position, 1.0f);
}
