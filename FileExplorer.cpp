#include "FileExplorerManager.h"
#include "FileExplorer.h"
#include "Folder.h"

FileExplorer::FileExplorer(const std::filesystem::path& absolutePath) : absolutePath(absolutePath) {
    window = sf::RenderWindow(sf::VideoMode({1000, 1000}), "Folder", sf::Style::None, sf::State::Windowed);

    folderPosition = {100, 100};
    for (const auto& entry : std::filesystem::directory_iterator(absolutePath)) {
        fileVec.emplace_back(std::make_unique<Custom::Folder>(sf::Vector2f(0, 0), entry.path()));
        fileVec.back()->sprite.setPosition(folderPosition);
        folderPosition.y += 200;
    }
    folderPosition = {100, 100};

    dragging = false;
    dragOffset = {0, 0};

}

void FileExplorer::update(FileExplorerManager& fileExplorerManager) {
    while (auto event = window.pollEvent()) {
        if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
            switch (mouseClickEvent->button) {
                case sf::Mouse::Button::Left: {
                    if (rect.getGlobalBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
                        dragging = true;
                        dragOffset = sf::Mouse::getPosition() - window.getPosition();
                    }
                    else {
                        for (auto& folder : fileVec)
                            if (folder->customBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
                                absolutePath = folder->absolutePath;
                                refresh();
                                break;
                            }
                    }
                    break;
                }
                case sf::Mouse::Button::Right: {
                    window.close();
                    break;
                }
                case sf::Mouse::Button::Middle: {
                    for (auto& folder : fileVec)
                        if (folder->customBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window))))
                            fileExplorerManager.fileExplorerVecBuffer.emplace_back(std::make_unique<FileExplorer>(FileExplorer(folder->absolutePath)));
                    break;
                }
            }
        }
        if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonReleased>()) {
            switch (mouseClickEvent->button) {
                case sf::Mouse::Button::Left: {
                    dragging = false;
                    break;
                }
            }
        }
        if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
            switch (keyEvent->code) {
                case sf::Keyboard::Key::W: break;
            }
        }
    }

    if (dragging) {
        window.setPosition(sf::Mouse::getPosition() - dragOffset);
    }

    window.clear(sf::Color::Black);
    window.draw(rect);
    for (auto& file : fileVec)
        window.draw(file->sprite);
    window.display();
}

void FileExplorer::refresh() {
    fileVec.clear();
    for (const auto& entry : std::filesystem::directory_iterator(absolutePath)) {
        fileVec.emplace_back(std::make_unique<Custom::Folder>(sf::Vector2f(0, 0), entry.path()));
        fileVec.back()->sprite.setPosition(folderPosition);
        folderPosition.y += 200;
    }
    folderPosition = {100, 100};
}