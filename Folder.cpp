#include "Folder.h"

Custom::Folder::Folder(const sf::Vector2f& position, const std::filesystem::path& absolutePath) : absolutePath(absolutePath), texture("..\\Files\\Placeholders\\placeholder100.png"), sprite(texture), font("..\\Files\\Font.TTF"), name(font) {
    sprite.setTexture(texture, true);
    sprite.setPosition(position);
    name.setFont(font);
    name.setPosition(position);
    name.setFillColor(sf::Color::Black);
    name.setString(absolutePath.filename().string());
}

void Custom::Folder::update(sf::RenderWindow& mainWindow) {
    mainWindow.draw(sprite);
    mainWindow.draw(name);
}

void Custom::Folder::refresh() {
    name.setString(absolutePath.filename().string());
}

sf::FloatRect Custom::Folder::customBounds() {
    sprite.scale(sf::Vector2f(1.f, 1.f));
    auto bounds = sprite.getGlobalBounds();
    sprite.scale(sf::Vector2f(1, 1));
    return bounds;
}