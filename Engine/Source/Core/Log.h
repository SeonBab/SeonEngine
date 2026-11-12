#pragma once

#include <string_view>

/** 로그 수준이다. Debug부터 Error까지 중요도가 높아진다. */
enum class LogLevel
{
	Debug,
	Info,
	Warning,
	Error
};

/** 엔진과 게임에서 함께 사용하는 로그 함수다. */
namespace Log
{
	/** 최소 출력 수준을 설정한다. 이보다 낮은 수준은 출력하지 않으며, 초기값은 Debug다. */
	void SetMinimumLevel(LogLevel level);

	/** 주어진 수준이 최소 출력 수준 이상이면 true를 반환한다. 로그를 출력하지는 않는다. */
	bool IsEnabled(LogLevel level);

	/**
	 * 완성된 메시지를 [분류] [수준] 메시지 형식으로 콘솔에 출력하고 줄바꿈을 붙인다.
	 * 최소 출력 수준보다 낮으면 출력하지 않는다. 색상을 지원하는 콘솔에서는 수준에만 색을 적용한다.
	 * 호출 중에 바로 출력하며 category와 message를 보관하지 않는다. 로그 수준과 관계없이 프로그램을 종료하지 않는다.
	 */
	void Write(LogLevel level, std::string_view category, std::string_view message);
}
