#pragma once

#include "Input/Key.h"
#include "Windows/WindowsHWrapper.h"

/** Windows 가상 키 코드를 엔진 키로 변환한다. 미지원 코드는 Invalid를 반환한다. */
EKey TranslateWindowsKey(WPARAM virtualKey);
