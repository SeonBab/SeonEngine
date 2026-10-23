#include "EngineLoop.h"

#include <cstdio>
#include <string_view>

int main(int argc, char* argv[])
{
	EEngineMode engineMode = EEngineMode::Editor;
	if (argc == 2 && std::string_view(argv[1]) == "-game")
	{
		engineMode = EEngineMode::Game;
	}
	else if (argc != 1)
	{
		std::fputs("Usage: SeonEngine.exe [-game]\n", stderr);

		return 1;
	}

	FEngineLoop engineLoop;

	return engineLoop.Run(engineMode);
}
