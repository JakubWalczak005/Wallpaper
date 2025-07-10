#pragma once
#include <SFML/Graphics.hpp>

namespace Custom {
    struct Folder;
}
struct FileExplorerManager;

struct FileExplorer {

    explicit FileExplorer(const std::filesystem::path& absolutePath);

    std::filesystem::path absolutePath;
    sf::RenderWindow window;
    std::vector<std::unique_ptr<Custom::Folder>> fileVec;

    sf::RectangleShape rect = sf::RectangleShape({1000, 50});
    bool dragging;
    sf::Vector2<int> dragOffset;
    sf::Vector2f folderPosition;

    void update(FileExplorerManager& fileExplorerManager);

    void refresh();

};