#pragma once

#include <filesystem>

#include <SFML/Graphics/Rect.hpp>

class AtlasGenerator
{
public:
	AtlasGenerator(const std::filesystem::path& aOutputDir);
	~AtlasGenerator();

	bool GenerateAtlas(const std::filesystem::path& aAtlasPath);

private:

	struct STexture
	{
		std::string id;
		sf::FloatRect rect;
	};

	const std::filesystem::path outputDir;
};