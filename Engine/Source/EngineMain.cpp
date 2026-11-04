#include "WindowsApplication.h"

int main()
{
	WindowsApplication windowsApplication;
	if (!windowsApplication.Initialize())
	{
		// @TODO 실패 로그
		return 1;
	}

	MSG message = {};

	while (true)
	{
		const BOOL result = GetMessageW(&message, nullptr, 0, 0);

		if (result == -1)
		{
			// @TODO 오류 로그
			return 1;
		}

		if (result == 0)
		{
			return static_cast<int>(message.wParam);
		}

		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return 0;
}