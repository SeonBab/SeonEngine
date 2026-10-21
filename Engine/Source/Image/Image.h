#pragma once

#include "Core/Containers/Array.h"
#include "Core/CoreTypes.h"

////////////////////////////////////////////////////////////////////////////////
// 단일 2D 이미지의 CPU RGBA8 픽셀을 소유한다. 기본 상태는 크기 0과 빈 배열이다.
// 위쪽 행부터 R/G/B/A 순서이며 행 패딩 없이 sizeX * sizeY * 4바이트다.
// 알파를 미리 곱하지 않으며 색상 공간 메타데이터, 파일 경로, GPU 자원은 보관하지 않는다.
////////////////////////////////////////////////////////////////////////////////
struct FImage
{
	int32 sizeX = 0;
	int32 sizeY = 0;
	TArray<uint8> rawData;
};
