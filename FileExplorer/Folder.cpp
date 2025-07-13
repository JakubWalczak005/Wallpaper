#include "Folder.h"
#include "../TextureManager.h"
#include "../FileLayout.h"

Custom::Folder::Folder(const std::filesystem::path& absolutePath, const FileLayout& fileLayout, const sf::Vector2f& position) : absolutePath(absolutePath), sprite(textureManager.fileTexVec["FolderIcon"]), title(absolutePath, fileLayout, position) {
    sprite.setOrigin(sf::Vector2f(sprite.getGlobalBounds().size.x / 2, sprite.getGlobalBounds().size.y / 2));
    sprite.setPosition(position);
    switch (fileLayout) {
        case FileLayout::LargeIcons: sprite.setScale({1, 1}); break;
        case FileLayout::Details: sprite.setScale({.3f, .3f}); break;
    }
}

void Custom::Folder::update(sf::RenderWindow& window) {
    window.draw(sprite);
    window.draw(title.sprite);
}

void Custom::Folder::setPosition(const sf::Vector2f& position) {
    sprite.move(position);
    title.sprite.move(position);
}

sf::FloatRect Custom::Folder::customBounds() {
    sprite.scale(sf::Vector2f(1.f, 1.f));
    auto bounds = sprite.getGlobalBounds();
    sprite.scale(sf::Vector2f(1, 1));
    return bounds;
}