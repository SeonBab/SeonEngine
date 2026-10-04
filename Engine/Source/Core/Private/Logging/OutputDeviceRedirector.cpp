#include "Logging/OutputDeviceRedirector.h"

#include <algorithm>

FOutputDeviceRedirector& FOutputDeviceRedirector::Get()
{
	// 처음 이 줄에 도달할 때 한 번만 만들어진다. 만드는 도중 다시 Get()을 부르면 안 되므로 생성자는 빈 목록만 준비한다
	static FOutputDeviceRedirector redirector;
	return redirector;
}

void FOutputDeviceRedirector::AddOutputDevice(IOutputDevice* outputDevice)
{
	// TODO(seon): assert를 만들면 nullptr과 중복 등록을 막는다. 중복이면 같은 로그가 두 번 출력된다
	outputDevices.push_back(outputDevice);
}

void FOutputDeviceRedirector::RemoveOutputDevice(IOutputDevice* outputDevice)
{
	const auto found = std::find(outputDevices.begin(), outputDevices.end(), outputDevice);
	// TODO(seon): assert를 만들면 등록하지 않은 장치의 해제를 막는다
	if (found != outputDevices.end())
	{
		outputDevices.erase(found);
	}
}

void FOutputDeviceRedirector::Write(const FLogCategory& category, ELogVerbosity verbosity, FStringView message)
{
	const FLogRecord record = {category, verbosity, message, std::chrono::system_clock::now()};
	for (IOutputDevice* outputDevice : outputDevices)
	{
		outputDevice->Write(record);
	}
}
