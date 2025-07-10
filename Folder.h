#pragma once
#include <SFML/Graphics.hpp>
#include <fmt/format.h>
#include <regex>

struct FileExplorer;

namespace Custom {
    struct Folder {

        Folder(const sf::Vector2f& position, const sf::Texture& texture, const std::string& absolutePath, sf::Font& font);

        sf::Sprite sprite;
        std::string absolutePath;
        sf::Text name;

        void update(sf::RenderWindow& mainWindow);

        sf::FloatRect customBounds();

    };
}