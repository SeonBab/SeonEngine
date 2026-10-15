#include "D3D11/D3D11Device.h"
#include "Windows/WindowsApplication.h"

int main()
{
	FD3D11Device graphics;
	if (!graphics.Initialize())
	{
		return 1;
	}

	const FWindowDescription description{};
	const int exitCode = RunWindowsApplication(description);
	graphics.Shutdown();
	return exitCode;
}
