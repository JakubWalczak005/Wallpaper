#include "Folder.h"

Custom::Folder::Folder(const sf::Vector2f& position, const sf::Texture& texture, const std::string& path) : sprite(texture), path(path.begin(), path.end()) {
    sprite.setPosition(position);
}

sf::FloatRect Custom::Folder::customBounds() {
    sprite.scale(sf::Vector2f(1.f, 1.f));
    auto bounds = sprite.getGlobalBounds();
    sprite.scale(sf::Vector2f(1, 1));
    return bounds;
}