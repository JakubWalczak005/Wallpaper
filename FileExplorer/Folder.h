#pragma once
#include <SFML/Graphics.hpp>
#include "Title.h"

namespace Custom {
    struct Folder {

        Folder(const std::filesystem::path& absolutePath, const FileLayout& fileLayout, const sf::Vector2f& position);

        std::filesystem::path absolutePath;
        sf::Sprite sprite;
        Title title;

        void update(sf::RenderWindow& window);

        void setPosition(const sf::Vector2f& position);

        sf::FloatRect customBounds();

    };
}