#include "TextureManager.h"
#include <fmt/base.h>

TextureManager textureManager;

TextureManager::TextureManager() {

    fileExplorerTexMap["Title"] = sf::Texture();

    fileTexVec["FolderIcon"] = sf::Texture("..\\Files\\Placeholders\\placeholder100.png");
    fileTexVec["CustomFont"] = sf::Texture("..\\Files\\CustomFont.png");

}