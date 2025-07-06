#pragma once
#include <SFML/Graphics.hpp>

namespace Custom {
    struct Folder {

        Folder(const sf::Vector2f& position, const sf::Texture& texture, const std::string& path);

        sf::Sprite sprite;
        std::wstring path;

        sf::FloatRect customBounds();

    };
}