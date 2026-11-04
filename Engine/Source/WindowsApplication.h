#pragma once

#include <Windows.h>

class WindowsApplication
{
public:
	bool Initialize();

private:
	bool Create();

private:
	static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

	HWND window = nullptr;
};

