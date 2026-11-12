#pragma once

#include <string>

/** 메시, 텍스처, 사운드 같은 엔진 콘텐츠의 공통 이름과 경로를 보관한다. */
class Asset
{
public:
	/**
	 * UTF-8 이름과 저장소 루트 기준 상대 경로를 보관한다. 이름은 중복될 수 있다.
	 * 파일 없이 생성한 에셋은 빈 경로를 사용한다. 경로 정규화와 파일 로딩은 호출하는 쪽에서 처리한다.
	 * 파일 안의 일부인 에셋은 파일 경로 뒤에 "#번호"가 붙어 실제 파일 경로가 아니다.
	 */
	Asset(std::string newName, std::string newPath);

	/** 기반 포인터로 삭제할 때 파생 클래스도 소멸시킨다. */
	virtual ~Asset() = default;

	/** 표시용 이름을 빌려준다. 반환한 참조는 이 객체가 살아 있는 동안 사용한다. */
	const std::string& GetName() const;

	/** 파일 없는 에셋은 빈 경로다. 반환한 참조는 이 객체가 살아 있는 동안 사용한다. */
	const std::string& GetPath() const;

private:
	std::string name;
	std::string path;
};
