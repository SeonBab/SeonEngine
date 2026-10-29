#pragma once

class FUIDrawElementList;
struct FUIImageDrawElement;
struct FUITextDrawElement;
struct FUIBatchData;

/** 이미지와 텍스트 요청을 API 독립적인 CPU 정점, 인덱스, 인접 배치로 변환한다. GPU 제출은 수행하지 않는다. */
class FUIElementBatcher
{
public:
	/**
	 * 목록 순서를 유지하며 이미지/글리프 사각형을 생성하고 같은 자원/샘플링의 인접 배치만 합친다.
	 * 이미지는 PointClamp, 글자는 LinearClamp다. 올바른 UTF-8을 받으며 중간 NUL과 탭은 거부한다.
	 * 공백은 펜 이동만, LF는 줄 이동, CR은 무시한다. 음수 글리프 오프셋을 보정하지 않는다.
	 * 빈 문자열도 폰트/좌표/색상/배율을 검사한다. 유효한 빈 문자열과 0 크기 글리프는 사각형을 만들지 않는다.
	 * 성공하면 출력을 교체하며 빈 목록은 빈 출력으로 성공한다. 잘못된 입력은 로그와 false이며 기존 출력은 유지한다.
	 * 입력 자산과 렌더 데이터는 호출과 이후 동기식 제출이 끝날 때까지 유지하고 교체하지 않아야 한다.
	 * 공통 메타데이터만 검사한다. 실제 GPU 준비, 백엔드와 장치 호환성은 제출부가 검사한다.
	 * 할당 예외는 전파한다. 실패한 호출의 이전 출력을 이번 결과로 제출해서는 안 된다.
	 */
	[[nodiscard]] bool Build(const FUIDrawElementList& elements, FUIBatchData& outData) const;

private:
	/** 후보에 이미지 한 장을 추가한다. 공통 사각형 생성부에서 개수/바이트 범위를 확인한다. */
	[[nodiscard]] bool AddImageGeometry(const FUIImageDrawElement& element, FUIBatchData& data) const;

	/** UTF-8을 코드 포인트로 읽고 공통 메트릭으로 배치한다. shaping/자동 줄바꿈은 수행하지 않는다. */
	[[nodiscard]] bool AddTextGeometry(const FUITextDrawElement& element, FUIBatchData& data) const;
};
