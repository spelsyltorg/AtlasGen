#include "AtlasGenerator.h"

#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Sprite.hpp"

#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>

AtlasGenerator::AtlasGenerator(const std::filesystem::path& aOutputDir) :
	outputDir(aOutputDir)
{
	const bool IsDirectory = std::filesystem::is_directory(aOutputDir);
	const bool Exists = std::filesystem::exists(aOutputDir);

	if (!IsDirectory && !Exists)
	{
		std::filesystem::create_directory(aOutputDir);
	}
}

AtlasGenerator::~AtlasGenerator()
{
}

bool AtlasGenerator::GenerateAtlas(const std::filesystem::path & aAtlasPath)
{
	struct IDPair
	{
		std::string id;
		sf::Texture* texture;
	};

	std::vector<IDPair> textures;

	const std::string name = aAtlasPath.stem().string();

	std::cout << "Initialized creation of atlas: " << name << std::endl;

	int minimumArea = 0;

	std::cout << "Loading textures..." << std::endl;

	for (auto&& texture : std::filesystem::directory_iterator(aAtlasPath))
	{
		auto&& file = texture.path();
		textures.emplace_back(IDPair{file.stem().string(), new sf::Texture() });
		sf::Texture& tex = *textures.back().texture;
		tex.loadFromFile(file.string());

		minimumArea += tex.getSize().x * tex.getSize().y;
	}

	std::cout << "Sorting files after area size..." << std::endl;

	std::sort(textures.begin(), textures.end(), [](auto& a, auto& b) {
		int areaA = a.texture->getSize().x * a.texture->getSize().y;
		int areaB = b.texture->getSize().x * b.texture->getSize().y;
		return areaA < areaB;
	});

	int currWidth = 0;
	int currHeight = 0;

	std::vector<std::pair<IDPair, sf::FloatRect>> placed;

	float minimumY = 0.f;

	while (!textures.empty())
	{
		sf::Texture& current = *textures.back().texture;

		// Try put on right, if not possible push down the least amount and drag to right until a place is found
		// try to keep width/height ratio near 1

		sf::FloatRect newPlacement;
		newPlacement.width = current.getSize().x;
		newPlacement.height = current.getSize().y;
		newPlacement.top = minimumY;

		if (currWidth == 0)
		{
			currWidth = newPlacement.width;
			currHeight = newPlacement.height;
		}

		for (int i = 0; i < placed.size(); ++i)
		{
			const auto& p = placed[i];
			if (newPlacement.intersects(p.second))
			{
				newPlacement.left = p.second.left + p.second.width;
				if (currWidth >= currHeight)
				{
					if (newPlacement.left + newPlacement.width >= currWidth)
					{
						minimumY = newPlacement.top + newPlacement.height;
					}
				}
				else
				{
					minimumY = 0.f;
				}
				i = 0;
			}
		}
		currWidth = std::max((float)currWidth, newPlacement.left + newPlacement.width);
		currHeight = std::max((float)currHeight, newPlacement.top + newPlacement.height);

		placed.emplace_back(std::make_pair(textures.back(), newPlacement));

		textures.pop_back();
	}

	int size = std::max(currWidth, currHeight);

	std::cout << "Creating atlas texture..." << std::endl;

	sf::RenderTexture atlasTexture;
	atlasTexture.create(size, size);

	std::cout << "Rendering textures onto atlas..." << std::endl;

	for (auto&& texture : placed)
	{
		sf::Sprite sprite;
		sprite.setTexture(*texture.first.texture);
		sprite.setPosition(texture.second.left, texture.second.top);
		atlasTexture.draw(sprite);
	}

	atlasTexture.display();

	std::cout << "Saving atlas texture to \"" << outputDir.string() + name + ".png..." << "\"" << std::endl;

	sf::Image atlasImage = atlasTexture.getTexture().copyToImage();
	atlasImage.saveToFile(outputDir.string() + name + ".png");

	std::cout << "Saving info to json..." << std::endl;

	std::ofstream file;

	file.open(outputDir.string() + name + ".json");

	file << "{" << std::endl;

	file << "\t\"textures\"" << " : [" << std::endl;

	for (auto&& p : placed)
	{
		std::string id = p.first.id;
		float x = p.second.left / (float)size;
		float y = p.second.top / (float)size;
		float w = p.second.width / (float)size;
		float h = p.second.height / (float)size;

		file << "\t\t{" << std::endl;
		file << "\t\t\t\"id\" : " << "\"" << id << "\"," << std::endl;
		file << "\t\t\t\"x\" : " << x << "," << std::endl;
		file << "\t\t\t\"y\" : " << y << "," << std::endl;
		file << "\t\t\t\"w\" : " << w << "," << std::endl;
		file << "\t\t\t\"h\" : " << h << std::endl;
		file << "\t\t}";
		if (&p != &placed.back()) {
			file << "," << std::endl;
		}
		else {
			file << std::endl;
		}
	}

	file << "\t]" << std::endl;

	file << "}" << std::endl;

	file.close();

	std::cout << "Done!" << std::endl << std::endl;

	return false;
}
