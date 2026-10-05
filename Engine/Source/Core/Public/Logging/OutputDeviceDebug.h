#pragma once

#include "Logging/OutputDevice.h"

// 디버거 출력 창(VS 출력 창 등)에 로그를 보내는 출력 장치다. Unreal FOutputDeviceDebug에 해당한다.
// 디버거가 없으면 보낸 글자는 아무 데도 나타나지 않는다.
class FOutputDeviceDebug final : public IOutputDevice
{
public:
	void Write(const FLogRecord& record) override;
};
