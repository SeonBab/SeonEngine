#pragma once

#include "Containers/Array.h"
#include "Logging/OutputDevice.h"

////////////////////////////////////////////////////////////////////////////////
// 로그를 등록된 출력 장치 모두에 전달한다. 엔진 전체에 하나만 있고 Get()으로 꺼낸다.
// 엔진은 싱글톤을 만들지 않지만 이 전달기는 예외다. 엔진 초기화 전과 Core 안에서도 로그를 남겨야 해서 엔진 객체가 소유하지 않는다.
// 출력 장치를 소유하지 않는다. 장치를 만든 쪽이 등록하고, 장치를 없애기 전에 해제한다.
////////////////////////////////////////////////////////////////////////////////
class FOutputDeviceRedirector
{
public:
	~FOutputDeviceRedirector() = default;

	// 장치 목록은 Get()이 돌려주는 객체 하나에만 있다. 복사본에 등록한 장치는 로그를 받지 못하므로 복사와 이동을 막는다
	FOutputDeviceRedirector(const FOutputDeviceRedirector&) = delete;
	FOutputDeviceRedirector& operator=(const FOutputDeviceRedirector&) = delete;
	FOutputDeviceRedirector(FOutputDeviceRedirector&&) = delete;
	FOutputDeviceRedirector& operator=(FOutputDeviceRedirector&&) = delete;

	/**
	 * 엔진 전체가 함께 쓰는 전달기를 돌려준다. 처음 부를 때 만들어진다.
	 * 프로그램이 끝나며 파괴된 뒤에는 부르지 않는다(전역 객체의 소멸자에서 로그를 남기지 않는다).
	 */
	static FOutputDeviceRedirector& Get();

	void AddOutputDevice(IOutputDevice* outputDevice);
	void RemoveOutputDevice(IOutputDevice* outputDevice);

	/** 지금 시각으로 기록을 만들어 등록된 출력 장치 모두에 넘긴다. 레벨 거르기는 SE_LOG가 이미 했다. */
	void Write(const FLogCategory& category, ELogVerbosity verbosity, FStringView message);

private:
	// Get() 밖에서 전달기를 따로 만들지 못하게 막는다
	FOutputDeviceRedirector() = default;

	TArray<IOutputDevice*> outputDevices;
};
