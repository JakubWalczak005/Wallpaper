#include "FileExplorer.h"
#include <fmt/base.h>

FileExplorer::FileExplorer(const std::string& absolutePath) : absolutePath(absolutePath) {
    window = sf::RenderWindow(sf::VideoMode({1000, 1000}), "Folder", sf::Style::None, sf::State::Windowed);

    for (const auto& entry : std::filesystem::directory_iterator(absolutePath)) {
        fmt::println("{}", entry.path().filename().string());
    }

    dragging = false;
    dragOffset = {0, 0};
}

void FileExplorer::update() {
    while (auto event = window.pollEvent()) {
        if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
            switch (mouseClickEvent->button) {
                case sf::Mouse::Button::Left: {
                    fmt::println("click");
                    if (rect.getGlobalBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
                        dragging = true;
                        dragOffset = sf::Mouse::getPosition() - window.getPosition();
                    }
                    break;
                }
                case sf::Mouse::Button::Right: {
                    window.close();
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
                case sf::Keyboard::Key::W: fmt::println("keyboard");
            }
        }
    }

    if (dragging) {
        window.setPosition(sf::Mouse::getPosition() - dragOffset);
    }

    window.clear(sf::Color::Black);
    window.draw(rect);
    window.display();
}