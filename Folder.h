#pragma once
#include <SFML/Graphics.hpp>

namespace Custom {
    struct Folder {

        Folder(const sf::Vector2f& position, const std::filesystem::path& absolutePath);

        std::filesystem::path absolutePath;
        sf::Texture texture;
        sf::Sprite sprite;
        sf::Font font;
        sf::Text name;

        void update(sf::RenderWindow& mainWindow);

        void refresh();

        sf::FloatRect customBounds();

    };
}