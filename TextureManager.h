#pragma once
#include <SFML/Graphics.hpp>

#include <map>

struct TextureManager {

    TextureManager();

    std::map<std::string, sf::Texture> fileExplorerTexMap;
    std::map<std::string, sf::Texture> fileTexVec;

};

extern TextureManager textureManager;

//change for vectors of textures
//move renderTextures here for performance reasons