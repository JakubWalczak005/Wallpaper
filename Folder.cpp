#include "Folder.h"

Custom::Folder::Folder(const sf::Vector2f& position, const sf::Texture& texture, const std::string& path, sf::Font& font) : sprite(texture), absolutePath(path.begin(), path.end()), name(font) {
    sprite.setPosition(position);
    name.setPosition(position);
    name.setFillColor(sf::Color::Black);
    std::smatch match;
    std::regex_search(path, match, std::regex("[^\\\\]+$"));
    name.setString(match[0].str());
}

void Custom::Folder::update(sf::RenderWindow& mainWindow) {
    mainWindow.draw(sprite);
    mainWindow.draw(name);
}

sf::FloatRect Custom::Folder::customBounds() {
    sprite.scale(sf::Vector2f(1.f, 1.f));
    auto bounds = sprite.getGlobalBounds();
    sprite.scale(sf::Vector2f(1, 1));
    return bounds;
}