#include "Title.h"
#include "Folder.h"
#include <fmt/base.h>
#include <algorithm>
#include "../TextureManager.h"
#include "../FileLayout.h"

Title::Title(const std::filesystem::path& absolutePath, const FileLayout& fileLayout, const sf::Vector2f& position) : tileSize(textureManager.fileTexVec["CustomFont"].getSize().x / 10), charSprite(textureManager.fileTexVec["CustomFont"]), sprite(renderTexture.getTexture()) {
    switch (fileLayout) {
        case FileLayout::LargeIcons: {
            auto tempRenderTextureVec = std::vector<sf::RenderTexture>();
            auto lineVec = wrapString(absolutePath.filename().string());
            for (auto& line : lineVec) {
                charSprite.setPosition({0, 0});
                if (!tempRenderTextureVec.emplace_back().resize({static_cast<unsigned>(tileSize + (line.size() - 1) * CHARACTER_SPACING.x), static_cast<unsigned>(tileSize)}))
                    fmt::println("render texture resize failed");
                for (auto& chr : line) {
                    charSprite.setTextureRect(charToTextureRect(chr));
                    tempRenderTextureVec.back().draw(charSprite);
                    charSprite.move({CHARACTER_SPACING.x, 0});
                }
                tempRenderTextureVec.back().display();
            }
            auto longestLineTextureWidth = std::ranges::max_element(tempRenderTextureVec, {}, [](const sf::RenderTexture& rt) {return rt.getSize().x;})->getSize().x;
            if (!renderTexture.resize({longestLineTextureWidth, static_cast<unsigned>(lineVec.size() * tileSize)}))
                fmt::println("render texture resize failed");
            auto offset = 0;
            for (auto& tempRenderTexture : tempRenderTextureVec) {
                auto tempSprite = sf::Sprite(tempRenderTexture.getTexture());
                tempSprite.setOrigin({tempSprite.getGlobalBounds().size.x / 2.f, tempSprite.getGlobalBounds().size.y / 2.f});
                tempSprite.setPosition({renderTexture.getSize().x / 2.f, renderTexture.getSize().y / 2.f - (lineVec.size() - 1) * (tileSize / 2.f)});
                tempSprite.move({0, static_cast<float>(offset)});
                renderTexture.draw(tempSprite);
                offset += CHARACTER_SPACING.y;
            }
            renderTexture.display();
            sprite.setTexture(renderTexture.getTexture(), true);
            sprite.setOrigin({sprite.getGlobalBounds().size.x / 2.f, sprite.getGlobalBounds().size.y / 2.f});
            sprite.setPosition({position.x - 1, position.y + 65 + (lineVec.size() - 1) * (tileSize / 2.f)});
        }
        break;
        case FileLayout::Details: {
            sprite.setPosition({position.x + 25, position.y});
            if (!renderTexture.resize({static_cast<unsigned>(tileSize + (absolutePath.filename().string().length() - 1) * tileSize / 2.f), static_cast<unsigned>(tileSize)}))
                fmt::println("render texture resize failed");

            for (auto& chr : absolutePath.filename().string()) {
                charSprite.setTextureRect(charToTextureRect(chr));
                renderTexture.draw(charSprite);
                charSprite.move({tileSize / 2.f, 0});
            }
            renderTexture.display();
            sprite.setTexture(renderTexture.getTexture(), true);
            sprite.setOrigin({0, sprite.getGlobalBounds().size.y / 2});
        }
        break;
    }
}

sf::IntRect Title::charToTextureRect(const char& chr) {
    auto index = static_cast<int>(chr) - 32;
    auto textureRect = sf::IntRect({0, 0}, {tileSize, tileSize});
    for (int i = 0; i < 10; ++i) {
        textureRect.position.x = 0;
        for (int j = 0; j < 10; ++j) {
            if (index <= 0) {
                return textureRect;
            }
            --index;
            textureRect.position.x += tileSize;
        }
        textureRect.position.y += tileSize;
    }
    return {};
}

std::vector<std::string> Title::wrapString(std::string string) {
    auto new_end = std::ranges::unique(string, [](char a, char b) {return a == ' ' && b == ' ';});
    string.erase(new_end.begin(), string.end());
    auto lineVec = std::vector<std::string>();
    for (int i = 0; i < string.size(); ++i) {
        if (string.at(i) == ' ' && i + 1 < string.size()) {
            lineVec.push_back(string.substr(0, i));
            string = string.substr(i + 1);
            i = -1;
        }
        else if (i + 1 < string.size() && string.substr(0, i).size() > LINE_CHARACTER_LIMIT) {
            lineVec.push_back(string.substr(0, i + 1));
            if (i + 2 < string.size() && string.at(i + 1) == ' ')
                string = string.substr(i + 2);
            else if (i + 1 < string.size())
                string = string.substr(i + 1);
            i = -1;
        }
    }
    if (!string.empty()) {
        lineVec.push_back(string);
    }
    return lineVec;
}