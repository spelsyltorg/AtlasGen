
#ifdef _DEBUG
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#else
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#endif

#include "AtlasGenerator.h"

#include <filesystem>
#include <iostream>

int main(int argc, char* args[])
{
	std::string atlasPath = "Atlases/";
	std::string atlasOutput = "Output/";

	if (argc > 1)
		atlasPath = args[1];
	if (argc > 2)
		atlasOutput = args[2];

	AtlasGenerator atlasGen(atlasOutput);

	std::filesystem::path atlasDir = atlasPath;
	if (std::filesystem::is_directory(atlasDir))
	{
		for (const auto& atlasPath : std::filesystem::directory_iterator(atlasDir))
		{
			atlasGen.GenerateAtlas(atlasPath.path());
		}
	}
	else
	{
		std::cout << "FAILURE: " << atlasDir.string() << " was not found." << std::endl;
	}
	std::cin.get();
}