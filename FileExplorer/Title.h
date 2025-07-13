#pragma once
#include "SFML/Graphics.hpp"
#include "../FileLayout.h"

struct Title {

    explicit Title(const std::filesystem::path& absolutePath, const FileLayout& fileLayout, const sf::Vector2f& position);

    //ADJUSTABLE
    const int LINE_CHARACTER_LIMIT = 20;
    const sf::Vector2f CHARACTER_SPACING = {12, 15};

    int tileSize;
    sf::Sprite charSprite;
    sf::RenderTexture renderTexture;
    sf::Sprite sprite;

    sf::IntRect charToTextureRect(const char& chr);

    std::vector<std::string> wrapString(std::string string);

};