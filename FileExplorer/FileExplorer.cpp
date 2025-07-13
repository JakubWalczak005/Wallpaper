#include "../FileExplorerManager.h"
#include "FileExplorer.h"
#include "Folder.h"
#include <fmt/base.h>

FileExplorer::FileExplorer(const std::filesystem::path& absolutePath) : absolutePath(absolutePath) {
    window = sf::RenderWindow(sf::VideoMode({1000, 1000}), "FileExplorer", sf::Style::None, sf::State::Windowed);
    backTemp.setFillColor(sf::Color::Black);
    fileTemp.setFillColor(sf::Color::Black);
    backTemp.setPosition({200, 0});
    dragging = false;
    dragOffset = {0, 0};
    fileScrollOffset = 0;
    resetAllFiles();
}

void FileExplorer::update(FileExplorerManager& fileExplorerManager) {
    while (auto event = window.pollEvent()) {
        if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
            switch (mouseClickEvent->button) {
                case sf::Mouse::Button::Left: {
                    if (barTemp.getGlobalBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
                        dragging = true;
                        dragOffset = sf::Mouse::getPosition() - window.getPosition();
                    }
                    if (backTemp.getGlobalBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
                        absolutePath = absolutePath.parent_path();
                        resetAllFiles();
                    }
                    if (fileTemp.getGlobalBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
                        switch (fileLayout) {
                            case FileLayout::LargeIcons: fileLayout = FileLayout::Details; break;
                            case FileLayout::Details: fileLayout = FileLayout::LargeIcons; break;
                        }
                        resetAllFiles();
                    }
                    for (auto& folder : fileVec)
                        if (folder->customBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
                            absolutePath = folder->absolutePath;
                            fileScrollOffset = 0;
                            resetAllFiles();
                            break;
                        }
                    break;
                }
                case sf::Mouse::Button::Right: {
                    window.close();
                    fileVec.clear();
                    fileExplorerManager.fileExplorerVecEraseBuffer.emplace_back(this);
                    break;
                }
                case sf::Mouse::Button::Middle: {
                    for (auto& folder : fileVec)
                        if (folder->customBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window))))
                            fileExplorerManager.fileExplorerVecAddBuffer.emplace_back(std::make_unique<FileExplorer>(FileExplorer(folder->absolutePath)));
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
        if (auto mouseWheelEvent = event->getIf<sf::Event::MouseWheelScrolled>()) {
            switch (mouseWheelEvent->wheel) {
                case sf::Mouse::Wheel::Vertical: {
                    fileScrollOffset += mouseWheelEvent->delta * SCROLL_SPEED;
                    offsetAllFiles({0, mouseWheelEvent->delta > 0 ? 20.f : -20.f});
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
    for (const auto& file : fileVec)
        file->update(window);
    window.draw(barTemp);
    window.draw(backTemp);
    window.draw(fileTemp);
    window.display();
}

void FileExplorer::offsetAllFiles(sf::Vector2f offset) {
    for (const auto& file : fileVec)
        file->setPosition(offset);
}

void FileExplorer::refresh() {
    auto freshFileVec = std::vector<std::filesystem::directory_entry>();
    for (const auto& entry : std::filesystem::directory_iterator(absolutePath))
        freshFileVec.push_back(entry);

    if (freshFileVec.size() != fileVec.size())
        fileVec.resize(freshFileVec.size());

    for (int i = 0; i < freshFileVec.size(); ++i)
        if (!fileVec.at(i) || freshFileVec.at(i) != fileVec.at(i)->absolutePath)
            fileVec.at(i) = std::make_unique<Custom::Folder>(freshFileVec.at(i), fileLayout, getFilePosition(i));
}

void FileExplorer::resetAllFiles() {
    fileVec.clear();
    for (const auto& entry : std::filesystem::directory_iterator(absolutePath)) {
        fileVec.emplace_back(std::make_unique<Custom::Folder>(entry.path(), fileLayout, sf::Vector2f(0, 0)));
    }
    setAllFilesPosition();
}

void FileExplorer::setAllFilesPosition() {
    switch (fileLayout) {
        case FileLayout::LargeIcons: {
            auto currentFilePosition = FIRST_FILE_POSITION_LARGE_ICONS;
            for (const auto& file : fileVec) {
                if (currentFilePosition.x < window.getSize().x) {
                    file->setPosition(currentFilePosition);
                    currentFilePosition.x += FILE_OFFSET_LARGE_ICONS.x;
                }
                else {
                    currentFilePosition = {FIRST_FILE_POSITION_LARGE_ICONS.x, currentFilePosition.y += FILE_OFFSET_LARGE_ICONS.y};
                    file->setPosition(currentFilePosition);
                    currentFilePosition = {currentFilePosition.x + FILE_OFFSET_LARGE_ICONS.x, currentFilePosition.y};
                }
            }
        }
        break;
        case FileLayout::Details: {
            auto currentFilePosition = FIRST_FILE_POSITION_DETAILS;
            for (const auto& file : fileVec) {
                file->setPosition(currentFilePosition);
                currentFilePosition.y += FILE_OFFSET_DETAILS;
            }
        }
        break;
    }
}

sf::Vector2f FileExplorer::getFilePosition(int index) {
    auto currentFilePosition = sf::Vector2f(0, 0);
    switch (fileLayout) {
        case FileLayout::LargeIcons: {
            currentFilePosition = FIRST_FILE_POSITION_LARGE_ICONS;
            for (; index > 0; --index) {
                if (currentFilePosition.x + FILE_OFFSET_LARGE_ICONS.x < window.getSize().x)
                    currentFilePosition.x += FILE_OFFSET_LARGE_ICONS.x;
                else {
                    currentFilePosition.x = FIRST_FILE_POSITION_LARGE_ICONS.x;
                    currentFilePosition.y += FILE_OFFSET_LARGE_ICONS.y;
                }
            }
        }
        break;
        case FileLayout::Details: {
            currentFilePosition = FIRST_FILE_POSITION_DETAILS;
            for (; index > 0; --index) {
                currentFilePosition.y += FILE_OFFSET_DETAILS;
            }
        }
    }
    return {currentFilePosition.x, currentFilePosition.y + fileScrollOffset};
}