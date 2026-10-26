#pragma once

#include <filesystem>

class FActorTypeRegistry;
class FComponentTypeRegistry;
class UWorld;

////////////////////////////////////////////////////////////////////////////////
// 맵 파일을 검사한 뒤 파일 순서대로 World에 액터를 추가한다.
////////////////////////////////////////////////////////////////////////////////
struct FMapLoader
{
	/**
	 * UTF-8 JSON 객체의 actors 배열과 각 액터의 components 배열을 검사한다. 두 배열은 비어 있어도 된다.
	 * 액터 type과 컴포넌트 name과 type은 비어 있지 않은 문자열이어야 하며 배열 항목은 객체여야 한다.
	 * 액터와 컴포넌트 type은 각각 전달한 등록표에 있어야 한다. 두 등록표는 호출 동안만 빌린다.
	 * 컴포넌트 이름은 대소문자를 구분해 같은 액터 안에서 유일해야 한다. 다른 액터의 이름과는 비교하지 않는다.
	 * name과 type 이외의 필드는 등록된 읽기 및 쓰기 프로퍼티여야 한다. 중복 지정은 거부한다.
	 * 프로퍼티는 유한한 float 범위의 숫자 세 개 배열이며 생략한 값은 유지한다. 회전 순서는 도 단위 pitch, yaw, roll이다.
	 * 상대 경로는 현재 작업 폴더 기준이다. 실제 NUL 바이트와 잘못된 UTF-8은 거부한다.
	 * 최초 파일 사전 검사 실패에서 경로와 원인을 로그로 남기고 false를 반환하며 World는 변경하지 않는다. 인덱스는 0부터 시작한다.
	 * 전체 검사 성공 후 생성 함수를 파일 순서대로 호출한다. 생성된 액터는 전달한 World가 소유하며 기존 액터는 유지한다.
	 * 액터마다 같은 이름의 기존 컴포넌트는 실제 타입이 일치하면 재사용하고, 없으면 생성해 이름과 함께 추가한다.
	 * JSON 필드 순서대로 getter의 값 타입에 맞춰 변환한 뒤 setter로 적용한다. 새 후보는 설정 성공 후 추가한다.
	 * 구성 중 타입 불일치, 생성이나 접근 및 추가 실패는 로그와 false를 반환하며 이미 생성하거나 적용한 내용을 유지한다.
	 * true는 검사와 액터 및 컴포넌트 구성과 프로퍼티 적용 완료를 뜻한다. 빈 actors 배열은 World 변경 없이 성공한다.
	 * 플레이 시작 전 초기 구성 과정에서 호출한다. 플레이 시작과 렌더 상태 생성은 수행하지 않는다.
	 * 파일, 버퍼, 문서와 생성 함수 목록은 함수 안에서만 유지한다. 생성 실패나 메모리 부족의 복구와 롤백은 제공하지 않는다.
	 */
	[[nodiscard]] static bool LoadMapFile(const std::filesystem::path& path, const FActorTypeRegistry& actorTypes, const FComponentTypeRegistry& componentTypes, UWorld& world);
};
