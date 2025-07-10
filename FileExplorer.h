#pragma once
#include "SFML/Graphics.hpp"
#include "Folder.h"

struct FileExplorer {

    explicit FileExplorer(const std::string& absolutePath);

    sf::RenderWindow window;
    std::string absolutePath;
    std::vector<Custom::Folder> files;

    sf::RectangleShape rect = sf::RectangleShape({1000, 50});
    bool dragging;
    sf::Vector2<int> dragOffset;

    void update();

};