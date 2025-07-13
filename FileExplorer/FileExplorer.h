#pragma once
#include <SFML/Graphics.hpp>
#include "../FileLayout.h"

namespace Custom {
    struct Folder;
}
struct FileExplorerManager;

struct FileExplorer {

    explicit FileExplorer(const std::filesystem::path& absolutePath);

    //ADJUSTABLE
    const int SCROLL_SPEED = 20;
    FileLayout fileLayout = FileLayout::LargeIcons;
    //LARGE ICONS
    const sf::Vector2f FIRST_FILE_POSITION_LARGE_ICONS = {100, 100};
    const sf::Vector2f FILE_OFFSET_LARGE_ICONS = {200, 200};
    //DETAILS
    const sf::Vector2f FIRST_FILE_POSITION_DETAILS = {100, 100};
    const int FILE_OFFSET_DETAILS = 40;

    std::filesystem::path absolutePath;
    sf::RenderWindow window;
    std::vector<std::unique_ptr<Custom::Folder>> fileVec;

    sf::RectangleShape barTemp = sf::RectangleShape({1000, 50});
    sf::RectangleShape backTemp = sf::RectangleShape({25, 25});
    sf::RectangleShape fileTemp = sf::RectangleShape({25, 25});

    bool dragging;
    sf::Vector2<int> dragOffset;
    float fileScrollOffset;

    void update(FileExplorerManager& fileExplorerManager);

    void offsetAllFiles(sf::Vector2f offset);

    void refresh();

    void resetAllFiles();

    void setAllFilesPosition();

    sf::Vector2f getFilePosition(int index);

};