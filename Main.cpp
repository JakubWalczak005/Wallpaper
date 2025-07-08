#include "Player.h"
#include "View.h"
#include "Weapons/Types/Debugger.h"
#include "Weapons/Types/CollisionTrigger.h"
#include "Weapons/Types/ThreadManager.h"
#include "Enemies/Types/SyntaxError.h"
#include "Enemies/Types/ExceptionThrower.h"
#include "Enemies/Types/Error.h"
#include "Enemies/Types/Knowledge.h"
#include "Collision.h"
#include "Effect.h"
#include "Utility/RNG.h"
#include "TileCover.h"
#include "Menu.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <fstream>
#include <fmt/ranges.h>

//PLEASE UNPACK FILES.ZIP AND PUT IT INSIDE THE UNPACKED 2DGAME FOLDER

/*
HOTKEYS
WASD -> MOVEMENT
E -> PICKUP
Q -> DROP
LSHIFT -> DASH
SPACE -> ZOOM OUT
LMB -> SHOOT
SCROLL OR 1,2,3 -> CYCLE THROUGH INVENTORY
ESCAPE -> MENU
F11 -> TOGGLE FPS
F12 -> TOGGLE PERFORMANCE MODE (FOR WEAKER MACHINES)
*/

std::vector<float> loadSave() {
    auto file = std::ifstream("..\\Files\\Save\\Save.txt");
    if (file) {
        auto vec = std::vector<float>();
        auto num = 0.f;
        while (file >> num)
            vec.push_back(num);
        return vec;
    }
    fmt::println("Save file is missing! Loading default state");
    return {500, 2400, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1650, 350, 4250, 400, 2400, 4350, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

auto main() -> int {

    //LOAD TEXTURES FOR:

    //MENUS
    auto menuTexMap = std::map<std::string, std::vector<sf::Texture>>();
    menuTexMap["Background"].push_back(sf::Texture("..\\Files\\Menus\\Background.png"));
    menuTexMap["PauseOverlay"].push_back(sf::Texture("..\\Files\\Menus\\PauseOverlay.png"));
    menuTexMap["MainBackground"].push_back(sf::Texture("..\\Files\\Menus\\MainBackground.png"));
    menuTexMap["PauseBackground"].push_back(sf::Texture("..\\Files\\Menus\\PauseBackground.png"));
    menuTexMap["Play"].push_back(sf::Texture("..\\Files\\Menus\\Play.png"));
    menuTexMap["Save"].push_back(sf::Texture("..\\Files\\Menus\\Save.png"));
    menuTexMap["Load"].push_back(sf::Texture("..\\Files\\Menus\\Load.png"));
    menuTexMap["Quit"].push_back(sf::Texture("..\\Files\\Menus\\Quit.png"));

    //MAP
    auto background = sf::Texture("..\\Files\\Map\\Background.png");
    auto backgroundSprite = sf::Sprite(background);
    backgroundSprite.setPosition(sf::Vector2f(-1000, -1000));

    auto tilePositionVec = std::vector<sf::Vector2f>();
    tilePositionVec.emplace_back(0, 2000);
    tilePositionVec.emplace_back(1000, 2000);
    tilePositionVec.emplace_back(0, 0);
    tilePositionVec.emplace_back(2000, 700);
    tilePositionVec.emplace_back(3500, 0);
    tilePositionVec.emplace_back(2000, 2200);
    tilePositionVec.emplace_back(1500, 3500);
    tilePositionVec.emplace_back(0, 3500);
    tilePositionVec.emplace_back(3300, 3500);
    auto tileTexMap = std::map<std::string, std::vector<sf::Texture>>();
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Map\\Floor\\Normal")) {
        const auto texture = sf::Texture(entry.path());
        tileTexMap["FloorNormal"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Map\\Floor\\Compiled")) {
        const auto texture = sf::Texture(entry.path());
        tileTexMap["FloorCompiled"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Map\\Wall\\Normal")) {
        const auto texture = sf::Texture(entry.path());
        tileTexMap["WallNormal"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Map\\Wall\\Compiled")) {
        const auto texture = sf::Texture(entry.path());
        tileTexMap["WallCompiled"].push_back(texture);
    }

    auto tileSpriteMap = std::map<std::string, std::vector<sf::Sprite>>();
    for (auto& texture : tileTexMap.at("FloorNormal"))
        tileSpriteMap["FloorNormal"].emplace_back(texture);
    for (int i = 0; i < tileSpriteMap["FloorNormal"].size(); ++i)
        tileSpriteMap["FloorNormal"].at(i).setPosition(tilePositionVec.at(i));
    for (auto& texture : tileTexMap.at("FloorCompiled"))
        tileSpriteMap["FloorCompiled"].emplace_back(texture);
    for (int i = 0; i < tileSpriteMap["FloorCompiled"].size(); ++i)
        tileSpriteMap["FloorCompiled"].at(i).setPosition(tilePositionVec.at(i));
    for (auto& texture : tileTexMap.at("WallNormal"))
        tileSpriteMap["WallNormal"].emplace_back(texture);
    for (int i = 0; i < tileSpriteMap["WallNormal"].size(); ++i)
        tileSpriteMap["WallNormal"].at(i).setPosition(tilePositionVec.at(i));
    for (auto& texture : tileTexMap.at("WallCompiled"))
        tileSpriteMap["WallCompiled"].emplace_back(texture);
    for (int i = 0; i < tileSpriteMap["WallCompiled"].size(); ++i)
        tileSpriteMap["WallCompiled"].at(i).setPosition(tilePositionVec.at(i));

    auto tileCoverTexMap = std::map<std::string, std::vector<sf::Texture>>();
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Map\\Cover\\Normal")) {
        const auto texture = sf::Texture(entry.path());
        tileCoverTexMap["CoverNormal"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Map\\Cover\\Compiled")) {
        const auto texture = sf::Texture(entry.path());
        tileCoverTexMap["CoverCompiled"].push_back(texture);
    }

    auto tileCoverSpriteMap = std::map<std::string, std::vector<TileCover>>();
    for (int i = 0; i < tileCoverTexMap["CoverNormal"].size(); ++i)
        tileCoverSpriteMap["CoverNormal"].emplace_back(tilePositionVec.at(i), tileCoverTexMap["CoverNormal"].at(i));
    for (int i = 0; i < tileCoverTexMap["CoverCompiled"].size(); ++i)
        tileCoverSpriteMap["CoverCompiled"].emplace_back(tilePositionVec.at(i), tileCoverTexMap["CoverCompiled"].at(i));


    //PLAYER
    auto playerTexMap = std::map<std::string, std::vector<sf::Texture>>();
    playerTexMap["Empty200"].push_back(sf::Texture("..\\Files\\Empties\\200.png"));
    playerTexMap["Empty1"].push_back(sf::Texture("..\\Files\\Empties\\1.png"));
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Animations\\Up")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["Up"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Animations\\Down")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["Down"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Animations\\Left")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["Left"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Animations\\Right")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["Right"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Animations\\DownRight")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["DownRight"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Animations\\DownLeft")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["DownLeft"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Animations\\UpRight")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["UpRight"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Animations\\UpLeft")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["UpLeft"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Player\\Level")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["Level"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Map\\Compile")) {
        const auto texture = sf::Texture(entry.path());
        playerTexMap["Compile"].push_back(texture);
    }
    playerTexMap["Integrity"].push_back(sf::Texture("..\\Files\\Player\\Integrity.png"));
    playerTexMap["Shadow"].push_back(sf::Texture("..\\Files\\Player\\Shadow.png"));
    playerTexMap["InventorySlotBackground"].push_back(sf::Texture("..\\Files\\Player\\InventorySlotBackground.png"));
    playerTexMap["HealthIcon"].push_back(sf::Texture("..\\Files\\Player\\HealthIcon.png"));
    playerTexMap["StaminaIcon"].push_back(sf::Texture("..\\Files\\Player\\StaminaIcon.png"));
    playerTexMap["InventoryIcon"].push_back(sf::Texture("..\\Files\\Player\\InventoryIcon.png"));
    playerTexMap["KnowledgeIcon"].push_back(sf::Texture("..\\Files\\Player\\KnowledgeIcon.png"));
    playerTexMap["GenericBarBackground"].push_back(sf::Texture("..\\Files\\Player\\GenericBarBackground.png"));
    playerTexMap["GenericBar"].push_back(sf::Texture("..\\Files\\Player\\GenericBar.png"));
    playerTexMap["GenericBarOutline"].push_back(sf::Texture("..\\Files\\Player\\GenericBarOutline.png"));
    playerTexMap["KnowledgeBarBackground"].push_back(sf::Texture("..\\Files\\Player\\KnowledgeBarBackground.png"));
    playerTexMap["KnowledgeBar"].push_back(sf::Texture("..\\Files\\Player\\KnowledgeBar.png"));
    playerTexMap["CompileButton"].push_back(sf::Texture("..\\Files\\Player\\CompileButton.png"));
    playerTexMap["WinButton"].push_back(sf::Texture("..\\Files\\Player\\WinButton.png"));
    playerTexMap["UnspentPoints"].push_back(sf::Texture("..\\Files\\Player\\UnspentPoints.png"));
    playerTexMap["Health"].push_back(sf::Texture("..\\Files\\Player\\Health.png"));
    playerTexMap["Stamina"].push_back(sf::Texture("..\\Files\\Player\\Stamina.png"));
    playerTexMap["Slot"].push_back(sf::Texture("..\\Files\\Player\\Slot.png"));

    //EFFECTS
    auto effectTexMap = std::map<std::string, std::vector<sf::Texture>>();
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\BinaryBlue")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["BinaryBlue"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Sleep")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["Sleep"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Compiled")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["Compiled"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Miss")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["Miss"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\NoStamina")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["NoStamina"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\NoKnowledge")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["NoKnowledge"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Knowledge")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["Knowledge"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\NoSlot")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["NoSlot"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\NoPickup")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["NoPickup"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\NoInventory")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["NoInventory"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Glitch")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["Glitch"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\LevelUp")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["LevelUp"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\MaxHealthLevel")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["MaxHealthLevel"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\MaxStaminaLevel")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["MaxStaminaLevel"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\MaxInventoryLevel")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["MaxInventoryLevel"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Saved")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["Saved"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Upgrade")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["Upgrade"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Projectiles\\Debugger")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["DebuggerProjectile"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Projectiles\\CollisionTrigger\\NoBounce")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["CollisionTriggerProjectile"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Projectiles\\CollisionTrigger\\Bounce")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["CollisionTriggerBounceProjectile"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Effects\\Projectiles\\ThreadManager")) {
        const auto texture = sf::Texture(entry.path());
        effectTexMap["ThreadManagerProjectile"].push_back(texture);
    }

    //ENEMIES
    auto enemyTexMap = std::map<std::string, std::vector<sf::Texture>>();
    enemyTexMap["Empty400"].push_back(sf::Texture("..\\Files\\Empties\\400.png"));
    enemyTexMap["SyntaxErrorSleep"].push_back(sf::Texture("..\\Files\\Enemies\\SyntaxError\\Sleep.png"));
    enemyTexMap["ExceptionThrowerSleep"].push_back(sf::Texture("..\\Files\\Enemies\\ExceptionThrower\\Sleep.png"));
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Enemies\\SyntaxError\\Idle")) {
        const auto texture = sf::Texture(entry.path());
        enemyTexMap["SyntaxErrorIdle"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Enemies\\SyntaxError\\Compile")) {
        const auto texture = sf::Texture(entry.path());
        enemyTexMap["SyntaxErrorCompile"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Enemies\\ExceptionThrower\\Idle")) {
        const auto texture = sf::Texture(entry.path());
        enemyTexMap["ExceptionThrowerIdle"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Enemies\\ExceptionThrower\\Compile")) {
        const auto texture = sf::Texture(entry.path());
        enemyTexMap["ExceptionThrowerCompile"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Enemies\\Error")) {
        const auto texture = sf::Texture(entry.path());
        enemyTexMap["Error"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Enemies\\Knowledge")) {
        const auto texture = sf::Texture(entry.path());
        enemyTexMap["Knowledge"].push_back(texture);
    }

    //WEAPONS
    auto weaponTexMap = std::map<std::string, std::vector<sf::Texture>>();
    weaponTexMap["Empty400"].push_back(sf::Texture("..\\Files\\Empties\\400.png"));
    weaponTexMap["Empty200"].push_back(sf::Texture("..\\Files\\Empties\\200.png"));
    weaponTexMap["Empty100"].push_back(sf::Texture("..\\Files\\Empties\\100.png"));
    weaponTexMap["Empty25"].push_back(sf::Texture("..\\Files\\Empties\\25.png"));
    weaponTexMap["PerformanceProjectile"].push_back(sf::Texture("..\\Files\\Weapons\\PerformanceProjectile.png"));
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Weapons\\Debugger\\Idle")) {
        const auto texture = sf::Texture(entry.path());
        weaponTexMap["DebuggerIdle"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Weapons\\Debugger\\IdleUpgraded")) {
        const auto texture = sf::Texture(entry.path());
        weaponTexMap["DebuggerIdleUpgraded"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Weapons\\CollisionTrigger\\Idle")) {
        const auto texture = sf::Texture(entry.path());
        weaponTexMap["CollisionTriggerIdle"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Weapons\\CollisionTrigger\\IdleUpgraded")) {
        const auto texture = sf::Texture(entry.path());
        weaponTexMap["CollisionTriggerIdleUpgraded"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Weapons\\ThreadManager\\Idle")) {
        const auto texture = sf::Texture(entry.path());
        weaponTexMap["ThreadManagerIdle"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Weapons\\ThreadManager\\IdleUpgraded")) {
        const auto texture = sf::Texture(entry.path());
        weaponTexMap["ThreadManagerIdleUpgraded"].push_back(texture);
    }
    for (const auto& entry : std::filesystem::directory_iterator("..\\Files\\Weapons\\ExceptionThrower\\Projectile")) {
        const auto texture = sf::Texture(entry.path());
        weaponTexMap["ExceptionThrowerProjectile"].push_back(texture);
    }

    auto wallVec = std::vector<std::unique_ptr<Collision>>();
    auto projectileVec = std::vector<std::unique_ptr<Projectile>>();
    auto effectVec = std::vector<std::unique_ptr<Effect>>();
    auto enemyVec = std::vector<std::unique_ptr<Enemy>>();
    auto weaponVec = std::vector<std::unique_ptr<Weapon>>();
    auto knowledgeVec = std::vector<std::unique_ptr<Knowledge>>();

    //PLACEHOLDERS (ONLY DEBUGGING)
    auto placeHolder100 = sf::Texture("..\\Files\\Placeholders\\placeholder100.png");
    auto placeHolder50 = sf::Texture("..\\Files\\Placeholders\\placeholder50.png");
    auto placeHolder25 = sf::Texture("..\\Files\\Placeholders\\placeholder25.png");
    auto placeHolder20 = sf::Texture("..\\Files\\Placeholders\\placeholder20.png");
    auto placeHolder10 = sf::Texture("..\\Files\\Placeholders\\placeholder10.png");

    auto collisionMap = sf::Image("..\\Files\\Map\\Collision.png");

    //PLACE WALLS
    auto mapSize = collisionMap.getSize().x;
    for (auto i = mapSize / 100; i < mapSize; i += mapSize / 50)
        for (auto j = mapSize / 100; j < collisionMap.getSize().y; j += mapSize / 50)
            if (collisionMap.getPixel(sf::Vector2u(i, j)).r > 100) {
                wallVec.push_back(std::make_unique<Collision>(Collision(sf::Vector2f(i - mapSize / 100, j - mapSize / 100), placeHolder100)));
            }

    auto player = Player(sf::Vector2f(500, 2400), playerTexMap, effectTexMap, effectVec);
    auto view = View();
    auto menu = Menu(menuTexMap);
    auto tileCleared = std::vector<bool>();
    auto saveState = std::vector<float>();

    auto deltaTime = sf::Clock();
    auto timeScale = 0.f;

    auto font = sf::Font("..\\Files\\arial\\ARIAL.TTF");
    auto fps = sf::Text(font);

    auto window = sf::RenderWindow(sf::VideoMode({1920, 1080}), "2DGame", sf::Style::Default, sf::State::Windowed, sf::ContextSettings{.antiAliasingLevel = 8});

    while (window.isOpen()) {

        //UPDATE VIEW
        if (!menu.pauseMenu) {
            view.view.setCenter(player.sprite.getPosition());
            view.update(timeScale);
            window.setView(view.view);
        }

        while (menu.mainMenu) {

            view.view.setSize(sf::Vector2f(1920, 1080));
            window.setView(view.view);
            menu.pauseMenu = false;

            if (menu.loadOverride) {
                saveState = loadSave();
                menu.loadOverride = false;
                menu.mainMenu = false;
            }

            while (auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>())
                    window.close();

                if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    switch (mouseClickEvent->button) {
                        case sf::Mouse::Button::Left:
                            menu.play.setScale(sf::Vector2f(.6f, .2f));
                            menu.load.setScale(sf::Vector2f(.6f, .2f));
                            menu.quit.setScale(sf::Vector2f(.6f, .2f));
                            if (menu.play.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                saveState = {500, 2400, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1650, 350, 4250, 400, 2400, 4350, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                                menu.mainMenu = false;
                            }
                            else if (menu.load.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                saveState = loadSave();
                                menu.mainMenu = false;
                            }
                            else if (menu.quit.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                saveState = {500, 2400, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1650, 350, 4250, 400, 2400, 4350, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                                window.close();
                                menu.mainMenu = false;
                            }
                            menu.play.setScale(sf::Vector2f(1, 1));
                            menu.load.setScale(sf::Vector2f(1, 1));
                            menu.quit.setScale(sf::Vector2f(1, 1));
                    }
                }
            }

            menu.update(view);
            window.clear(sf::Color::Black);
            for (auto& mainMenuElement : menu.mainMenuVec)
                window.draw(*mainMenuElement);
            window.display();

            if (!menu.mainMenu) {

                player.inventory.clear();
                tileCleared.clear();
                projectileVec.clear();
                effectVec.clear();
                enemyVec.clear();
                weaponVec.clear();
                knowledgeVec.clear();

                //VIEW
                view.zoomOut = false;

                //PLAYER
                player.movingLeft = false;
                player.movingRight = false;
                player.movingUp = false;
                player.movingDown = false;
                player.dashing = false;
                player.shooting = false;
                player.sprite.setPosition(sf::Vector2f(saveState.at(0), saveState.at(1)));
                player.sprite.setScale(sf::Vector2f(.25f, .25f));
                player.shadowSprite.setScale(sf::Vector2f(1, 1));
                player.health = saveState.at(2);
                player.setHealthLevel(saveState.at(3));
                player.equippedSlot = saveState.at(4);
                player.inventoryCapacity = saveState.at(5);
                player.stamina = saveState.at(6);
                player.setStaminaLevel(saveState.at(7));
                player.knowledge = saveState.at(8);
                player.setKnowledgeLevel(saveState.at(9));
                player.upgradePoints = saveState.at(10);

                for (int i = 11; i < 14; ++i) {
                    switch (static_cast<int>(saveState.at(i))) {
                        case 1: player.inventory.push_back(std::make_unique<Debugger>(Debugger(sf::Vector2f(0, 0), weaponTexMap, effectTexMap, projectileVec, effectVec)));break;
                        case 2: player.inventory.push_back(std::make_unique<CollisionTrigger>(CollisionTrigger(sf::Vector2f(0, 0), weaponTexMap,  effectTexMap, projectileVec, effectVec))); break;
                        case 3: player.inventory.push_back(std::make_unique<ThreadManager>(ThreadManager(sf::Vector2f(0, 0), weaponTexMap, effectTexMap, projectileVec, effectVec))); break;
                    }
                }
                for (auto& weapon : player.inventory)
                    weapon->pickedUp = true;

                if (saveState.at(14) != 0)
                    weaponVec.push_back(std::make_unique<Debugger>(Debugger(sf::Vector2f(saveState.at(14), saveState.at(15)), weaponTexMap, effectTexMap, projectileVec, effectVec)));
                if (saveState.at(16) != 0)
                    weaponVec.push_back(std::make_unique<CollisionTrigger>(CollisionTrigger(sf::Vector2f(saveState.at(16), saveState.at(17)), weaponTexMap, effectTexMap, projectileVec, effectVec)));
                if (saveState.at(18) != 0)
                    weaponVec.push_back(std::make_unique<ThreadManager>(ThreadManager(sf::Vector2f(saveState.at(18), saveState.at(19)), weaponTexMap, effectTexMap, projectileVec, effectVec)));

                //TILE 1
                if (saveState.at(20) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(500, 2800), enemyTexMap, effectTexMap, effectVec, 1)));
                }

                //TILE 2
                if (saveState.at(21) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(1500, 2400), enemyTexMap, effectTexMap, effectVec, 2)));
                }

                //TILE 3
                if (saveState.at(22) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(1000, 1000), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 1000, 1, 3)));
                    for (int i = 0; i < 5; ++i) {
                        switch (RNG::randomInt(1, 8)) {
                            case 1: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(200, 500), RNG::randomFloat(200, 1800)), enemyTexMap, effectTexMap, effectVec, 3))); break;
                            case 2: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(200, 1800), RNG::randomFloat(200, 500)), enemyTexMap, effectTexMap, effectVec, 3))); break;
                            case 3: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(200, 1800), RNG::randomFloat(1500, 1800)), enemyTexMap, effectTexMap, effectVec, 3))); break;
                            case 4: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(1500, 1800), RNG::randomFloat(800, 1800)), enemyTexMap, effectTexMap, effectVec, 3))); break;
                            case 5: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(200, 500), RNG::randomFloat(200, 1800)), enemyTexMap, effectTexMap, effectVec, 3))); break;
                            case 6: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(200, 1800), RNG::randomFloat(200, 500)), enemyTexMap, effectTexMap, effectVec, 3))); break;
                            case 7: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(200, 1800), RNG::randomFloat(1500, 1800)), enemyTexMap, effectTexMap, effectVec, 3))); break;
                            case 8: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(1500, 1800), RNG::randomFloat(800, 1800)), enemyTexMap, effectTexMap, effectVec, 3))); break;
                        }
                    }
                }

                //TILE 4
                if (saveState.at(23) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    enemyVec.push_back(std::make_unique<SyntaxError>(SyntaxError(sf::Vector2f(3200, 1000), enemyTexMap, effectTexMap, effectVec, 500, CollisionSide::Right, 1, 4)));
                    enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(3150, 1750), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 750, 4)));
                    for (int i = 0; i < 3; ++i) {
                        switch (RNG::randomInt(1, 2)) {
                            case 1: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(2200, 2800), RNG::randomFloat(900, 2000)), enemyTexMap, effectTexMap, effectVec, 4))); break;
                            case 2: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(2200, 2800), RNG::randomFloat(800, 2100)), enemyTexMap, effectTexMap, effectVec, 4))); break;
                        }
                    }
                }

                //TILE 5
                if (saveState.at(24) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(3800, 300), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 1000, 5)));
                    enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(4700, 300), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 1000, 5)));
                    enemyVec.push_back(std::make_unique<SyntaxError>(SyntaxError(sf::Vector2f(4000, 1400), enemyTexMap, effectTexMap, effectVec, 500, static_cast<CollisionSide>(RNG::randomInt(1, 4)), 5)));
                    enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(4300, 2300), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 500, 5)));
                    for (int i = 0; i < 5; ++i) {
                        switch (RNG::randomInt(1, 4)) {
                            case 1: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(3700, 4800), RNG::randomFloat(500, 900)), enemyTexMap, effectTexMap, effectVec, 5))); break;
                            case 2: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(3700, 4800), RNG::randomFloat(500, 900)), enemyTexMap, effectTexMap, effectVec, 5))); break;
                            case 3: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(4300, 4800), RNG::randomFloat(1400, 2500)), enemyTexMap, effectTexMap, effectVec, 5))); break;
                            case 4: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(4300, 4800), RNG::randomFloat(1400, 2500)), enemyTexMap, effectTexMap, effectVec, 5))); break;
                        }
                    }
                }

                //TILE 6
                if (saveState.at(25) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(2400, 2450), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 1500, 6)));
                    enemyVec.push_back(std::make_unique<SyntaxError>(SyntaxError(sf::Vector2f(2400, 3400), enemyTexMap, effectTexMap, effectVec, 1500, static_cast<CollisionSide>(RNG::randomInt(1, 4)), 6)));
                    for (int i = 0; i < 2; ++i) {
                        switch (RNG::randomInt(1, 2)) {
                            case 1: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(2200, 2600), RNG::randomFloat(2700, 3000)), enemyTexMap, effectTexMap, effectVec, 6))); break;
                            case 2: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(2200, 2600), RNG::randomFloat(2700, 3000)), enemyTexMap, effectTexMap, effectVec, 6))); break;
                        }
                    }
                }

                //TILE 7
                if (saveState.at(26) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(1800, 4700), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 500, 7)));
                    enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(3000, 4700), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 500, 7)));
                    for (int i = 0; i < 5; ++i) {
                        switch (RNG::randomInt(1, 2)) {
                            case 1: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(1700, 3100), RNG::randomFloat(3700, 4100)), enemyTexMap, effectTexMap, effectVec, 7))); break;
                            case 2: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(1700, 3100), RNG::randomFloat(3700, 4100)), enemyTexMap, effectTexMap, effectVec, 7))); break;
                        }
                    }
                }

                //TILE 8
                if (saveState.at(27) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    for (int i = 0; i < 10; ++i) {
                        switch (RNG::randomInt(1, 4)) {
                            case 1: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(200, 1300), RNG::randomFloat(3700, 4800)), enemyTexMap, effectTexMap, effectVec, 8))); break;
                            case 2: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(200, 1300), RNG::randomFloat(3700, 4800)), enemyTexMap, effectTexMap, effectVec, 8))); break;
                            case 3: enemyVec.push_back(std::make_unique<SyntaxError>(SyntaxError(sf::Vector2f(RNG::randomFloat(200, 1300), RNG::randomFloat(3700, 4800)), enemyTexMap, effectTexMap, effectVec, 500, static_cast<CollisionSide>(RNG::randomInt(1, 4)), 8))); break;
                            case 4: enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(RNG::randomFloat(200, 1300), RNG::randomFloat(3700, 4800)), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 500, 8))); break;
                        }
                    }
                }

                //TILE 9
                if (saveState.at(28) == 1)
                    tileCleared.push_back(true);
                else {
                    tileCleared.push_back(false);
                    for (int i = 0; i < 10; ++i) {
                        switch (RNG::randomInt(1, 4)) {
                            case 1: enemyVec.push_back(std::make_unique<Error>(Error(sf::Vector2f(RNG::randomFloat(3500, 4800), RNG::randomFloat(3700, 4800)), enemyTexMap, effectTexMap, effectVec, 9))); break;
                            case 2: knowledgeVec.push_back(std::make_unique<Knowledge>(Knowledge(sf::Vector2f(RNG::randomFloat(3500, 4800), RNG::randomFloat(3700, 4800)), enemyTexMap, effectTexMap, effectVec, 9))); break;
                            case 3: enemyVec.push_back(std::make_unique<SyntaxError>(SyntaxError(sf::Vector2f(RNG::randomFloat(3500, 4800), RNG::randomFloat(3700, 4800)), enemyTexMap, effectTexMap, effectVec, 500, static_cast<CollisionSide>(RNG::randomInt(1, 4)), 9))); break;
                            case 4: enemyVec.push_back(std::make_unique<ExceptionThrower>(ExceptionThrower(sf::Vector2f(RNG::randomFloat(3500, 4800), RNG::randomFloat(3700, 4800)), enemyTexMap, weaponTexMap, effectTexMap, projectileVec, effectVec, 500, 9))); break;
                        }
                    }
                }

            }
            deltaTime.restart();
        }

        timeScale = 60 * deltaTime.restart().asSeconds();

        while (auto event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>())
                window.close();

            if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyEvent->code) {
                    case sf::Keyboard::Key::W: player.movingUp = true; break;
                    case sf::Keyboard::Key::A: player.movingLeft = true; break;
                    case sf::Keyboard::Key::S: player.movingDown = true; break;
                    case sf::Keyboard::Key::D: player.movingRight = true; break;
                    case sf::Keyboard::Key::F1: window.setFramerateLimit(0); break;
                    case sf::Keyboard::Key::F2: window.setFramerateLimit(60); break;
                    case sf::Keyboard::Key::F3: window.setFramerateLimit(30); break;
                    case sf::Keyboard::Key::F4: window.setFramerateLimit(10); break;
                    case sf::Keyboard::Key::F11: player.showFPS = !player.showFPS; break;
                    case sf::Keyboard::Key::F12: player.performanceMode = !player.performanceMode; break;
                    case sf::Keyboard::Key::E:

                        player.emptyGround = true;
                        for (auto& knowledge : knowledgeVec) {
                            if (player.checkCollision(*knowledge)) {
                                player.emptyGround = false;
                                player.increaseKnowledge(.1f);
                                player.increaseHealth(.3f);
                                effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("Knowledge"))));
                                knowledge->alive = false;
                                break;
                            }
                        }
                        if (!player.emptyGround)
                            break;

                        if (player.inventory.size() < player.inventoryCapacity) {
                            for (auto it = weaponVec.begin(); it != weaponVec.end(); ++it) {
                                if (player.checkCollision(**it)) {
                                    player.emptyGround = false;
                                    (*it)->pickedUp = true;
                                    player.pickUp(std::move(*it));
                                    weaponVec.erase(it);
                                    break;
                                }
                            }
                            if (player.emptyGround)
                                effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("NoPickup"))));
                        }
                        if (player.inventory.size() >= player.inventoryCapacity)
                            effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("NoInventory"))));

                    break;

                    case sf::Keyboard::Key::Q:

                        if (player.equippedSlot < player.inventory.size()) {
                            player.drop(weaponVec);
                        }
                        else
                            effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("NoSlot"))));

                    break;

                    case sf::Keyboard::Key::LShift:

                        if (player.stamina >= .3f) {
                            player.stamina -= .3f;
                            player.dash(2);
                        }
                        else
                            effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("NoStamina"))));

                    break;

                    case sf::Keyboard::Key::Space: view.zoomOut = true; break;
                    case sf::Keyboard::Key::Num1: player.equippedSlot = 0; break;
                    case sf::Keyboard::Key::Num2: if (player.inventoryCapacity >= 2) player.equippedSlot = 1; break;
                    case sf::Keyboard::Key::Num3: if (player.inventoryCapacity >= 3) player.equippedSlot = 2; break;
                    case sf::Keyboard::Key::Escape: if (view.showOverlay) menu.pauseMenu = !menu.pauseMenu; break;
                }
            }
            if (auto keyEvent = event->getIf<sf::Event::KeyReleased>()) {
                switch (keyEvent->code) {
                    case sf::Keyboard::Key::W: player.movingUp = false; break;
                    case sf::Keyboard::Key::A: player.movingLeft = false; break;
                    case sf::Keyboard::Key::S: player.movingDown = false; break;
                    case sf::Keyboard::Key::D: player.movingRight = false; break;
                    case sf::Keyboard::Key::Space: view.zoomOut = false; break;
                    case sf::Keyboard::Key::LShift: player.dashing = false; break;
                }
            }
            if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                switch (mouseClickEvent->button) {
                    case sf::Mouse::Button::Left:
                        if (!view.zoomOut) {
                            player.healthIcon.setScale(sf::Vector2f(.5f, .5f));
                            player.staminaIcon.setScale(sf::Vector2f(.5f, .5f));
                            player.inventoryIcon.setScale(sf::Vector2f(.5f, .5f));
                            player.compileButton.setScale(sf::Vector2f(.5f, .5f));
                            player.winButton.setScale(sf::Vector2f(.25f, .25f));
                            player.healthSprite.setScale(sf::Vector2f(.5f, .5f));
                            player.staminaSprite.setScale(sf::Vector2f(.5f, .5f));
                            player.slotSprite.setScale(sf::Vector2f(.5f, .5f));

                            if (player.healthIcon.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) or player.healthSprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                if (player.healthLevel <= 2) {
                                    if (player.upgradePoints > 0) {
                                        view.shake(10);
                                        player.increaseHealthLevel();
                                        player.increaseHealth(.5f);
                                        effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("Upgrade"))));
                                    }
                                    else
                                        effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("NoKnowledge"))));

                                }
                                else
                                    effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("MaxHealthLevel"))));
                            }
                            else if (player.staminaIcon.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) or player.staminaSprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                if (player.staminaLevel <= 2) {
                                    if (player.upgradePoints > 0) {
                                        view.shake(10);
                                        player.increaseStaminaLevel();
                                        effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("Upgrade"))));
                                    }
                                    else
                                        effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("NoKnowledge"))));
                                }
                                else
                                    effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("MaxStaminaLevel"))));
                            }
                            else if (player.inventoryIcon.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) or player.slotSprite.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                if (player.inventoryCapacity <= 2) {
                                    if (player.upgradePoints > 0) {
                                        view.shake(10);
                                        ++player.inventoryCapacity;
                                        --player.upgradePoints;
                                        effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("Upgrade"))));
                                    }
                                    else
                                        effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("NoKnowledge"))));
                                }
                                else
                                    effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("MaxInventoryLevel"))));

                            }
                            else if (player.compileButton.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) and std::ranges::find_if(enemyVec, [&](const auto& enemy){ return enemy->tile == player.tile;}) == enemyVec.end() and !tileCleared.at(player.tile - 1)) {
                                view.shake(10);
                                player.increaseKnowledge(.3f);
                                effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("Knowledge"))));
                                effectVec.emplace_back(std::make_unique<Effect>(Effect(sf::Vector2f(tileSpriteMap.at("FloorCompiled").at(player.tile - 1).getPosition().x + (tileSpriteMap.at("FloorCompiled").at(player.tile - 1).getGlobalBounds().size.x / 2), tileSpriteMap.at("FloorCompiled").at(player.tile - 1).getPosition().y + (tileSpriteMap.at("FloorCompiled").at(player.tile - 1).getGlobalBounds().size.y / 2)), effectTexMap.at("Compiled"))));
                                tileCleared.at(player.tile - 1) = true;
                            }
                            else if (player.winButton.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) and std::ranges::all_of(tileCleared.begin(), tileCleared.end(), [](int x) {return x == 1;})) {
                                player.animationSpeed = 0;
                                player.animationFrame = 0;
                                player.sprite.setScale(sf::Vector2f(1, 1));
                                player.sprite.setTexture(playerTexMap.at("Compile").at(0));
                                while (window.isOpen()) {
                                    player.animationSpeed += timeScale * .1f;
                                    while (player.animationSpeed >= 1) {
                                        player.animationSpeed -= 1;
                                        player.sprite.setTexture(playerTexMap.at("Compile").at(player.animationFrame++));
                                        if (player.animationFrame >= playerTexMap.at("Compile").size()) {
                                            player.animationSpeed = 0;
                                            player.animationFrame = 0;
                                            window.close();
                                        }
                                    }
                                    window.clear();
                                    window.draw(player.sprite);
                                    window.display();
                                }
                            }
                            else
                                player.shooting = true;

                            player.healthIcon.setScale(sf::Vector2f(1, 1));
                            player.staminaIcon.setScale(sf::Vector2f(1, 1));
                            player.inventoryIcon.setScale(sf::Vector2f(1, 1));
                            player.compileButton.setScale(sf::Vector2f(1, 1));
                            player.winButton.setScale(sf::Vector2f(1, 1));
                            player.healthSprite.setScale(sf::Vector2f(1, 1));
                            player.staminaSprite.setScale(sf::Vector2f(1, 1));
                            player.slotSprite.setScale(sf::Vector2f(1, 1));
                        }
                        else
                            player.shooting = true;
                        if (menu.pauseMenu) {
                            menu.save.setScale(sf::Vector2f(.6f, .2f));
                            menu.load.setScale(sf::Vector2f(.6f, .2f));
                            menu.quit.setScale(sf::Vector2f(.6f, .2f));
                            if (menu.save.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                effectVec.emplace_back(std::make_unique<Effect>(Effect(sf::Vector2f(menu.save.getPosition().x, menu.save.getPosition().y - 120), effectTexMap.at("Saved"))));
                                saveState.clear();
                                saveState.push_back(player.sprite.getPosition().x);
                                saveState.push_back(player.sprite.getPosition().y);
                                saveState.push_back(player.health);
                                saveState.push_back(player.healthLevel);
                                saveState.push_back(player.equippedSlot);
                                saveState.push_back(player.inventoryCapacity);
                                saveState.push_back(player.stamina);
                                saveState.push_back(player.staminaLevel);
                                saveState.push_back(player.knowledge);
                                saveState.push_back(player.knowledgeLevel);
                                saveState.push_back(player.upgradePoints);
                                for (auto& weapon : player.inventory) {
                                    if (auto* debugger = dynamic_cast<Debugger*>(weapon.get()); debugger)
                                        saveState.push_back(1);
                                    else if (auto* collisionTrigger = dynamic_cast<CollisionTrigger*>(weapon.get()); collisionTrigger)
                                        saveState.push_back(2);
                                    else if (auto* threadManager = dynamic_cast<ThreadManager*>(weapon.get()); threadManager)
                                        saveState.push_back(3);
                                }
                                for (int i = 3; i > player.inventory.size(); --i) {
                                    saveState.push_back(0);
                                }
                                for (int i = 0; i < 6; ++i)
                                    saveState.push_back(0);
                                for (auto& weapon : weaponVec) {
                                    if (auto* debugger = dynamic_cast<Debugger*>(weapon.get()); debugger) {
                                        saveState.at(14) = debugger->sprite.getPosition().x;
                                        saveState.at(15) = debugger->sprite.getPosition().y;
                                    }
                                    if (auto* collisionTrigger = dynamic_cast<CollisionTrigger*>(weapon.get()); collisionTrigger) {
                                        saveState.at(16) = collisionTrigger->sprite.getPosition().x;
                                        saveState.at(17) = collisionTrigger->sprite.getPosition().y;
                                    }
                                    if (auto* threadManager = dynamic_cast<ThreadManager*>(weapon.get()); threadManager) {
                                        saveState.at(18) = threadManager->sprite.getPosition().x;
                                        saveState.at(19) = threadManager->sprite.getPosition().y;
                                    }
                                }
                                for (auto tile : tileCleared)
                                    if (tile)
                                        saveState.push_back(1);
                                    else
                                        saveState.push_back(0);

                                auto file = std::ofstream("..\\Files\\Save\\Save.txt");
                                file.close();
                                file = std::ofstream("..\\Files\\Save\\Save.txt", std::ios::app);
                                if (file)
                                    for (auto& saveElement : saveState)
                                        file << saveElement << " ";
                            }
                            else if (menu.load.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))) {
                                menu.mainMenu = true;
                                menu.loadOverride = true;
                            }
                            else if (menu.quit.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))))
                                menu.mainMenu = true;
                            menu.save.setScale(sf::Vector2f(1, 1));
                            menu.load.setScale(sf::Vector2f(1, 1));
                            menu.quit.setScale(sf::Vector2f(1, 1));
                        }

                    break;
                }
            }
            if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonReleased>()) {
                switch (mouseClickEvent->button) {
                    case sf::Mouse::Button::Left: player.shooting = false;
                    for (auto& weapon : player.inventory)
                        weapon->timeThreshold = 1;
                    break;
                }
            }
            if (auto mouseWheelEvent = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (mouseWheelEvent->delta > 0)
                    player.inventorySlotForward();
                else if (mouseWheelEvent->delta < 0)
                    player.inventorySlotBackward();
            }
        }

        //UPDATE ENTITIES
        if (!menu.pauseMenu)
            player.update(timeScale, window, view.view);

        //COLLISION CHECKS (TILE, PLAYER, WALL, DEBUGGER, COLLISION TRIGGER, THREAD MANAGER, SYNTAX ERROR, EXCEPTION THROWER, ERROR, KNOWLEDGE)

        //PLAYER -> TILE
        for (int i = 0; i < tilePositionVec.size(); ++i) {
            if (player.tileCheck.checkCollision(tileCoverSpriteMap["CoverNormal"].at(i))) {
                player.tile = i + 1;
                if (tileCleared.at(i)) {
                    tileCoverSpriteMap["CoverCompiled"].at(i).decreaseOpacity(timeScale * 10);
                    tileCoverSpriteMap["CoverCompiled"].at(i).sprite.setColor(sf::Color(255, 255, 255, tileCoverSpriteMap["CoverCompiled"].at(i).opacity));
                }
                else {
                    tileCoverSpriteMap["CoverNormal"].at(i).decreaseOpacity(timeScale * 10);
                    tileCoverSpriteMap["CoverNormal"].at(i).sprite.setColor(sf::Color(255, 255, 255, tileCoverSpriteMap["CoverNormal"].at(i).opacity));
                }
            }
            else {
                tileCoverSpriteMap["CoverNormal"].at(i).increaseOpacity(timeScale * 10);
                tileCoverSpriteMap["CoverNormal"].at(i).sprite.setColor(sf::Color(255, 255, 255, tileCoverSpriteMap["CoverNormal"].at(i).opacity));
                tileCoverSpriteMap["CoverCompiled"].at(i).increaseOpacity(timeScale * 10);
                tileCoverSpriteMap["CoverCompiled"].at(i).sprite.setColor(sf::Color(255, 255, 255, tileCoverSpriteMap["CoverCompiled"].at(i).opacity));
            }
        }
        //WITH WALL
        for (auto& wall: wallVec) {
            //PLAYER -> WALL
            if (!player.performanceMode) {
                if (player.checkCollision(*wall)) {
                    if (player.side == CollisionSide::Top or player.side == CollisionSide::Bottom) {
                        player.sprite.move(sf::Vector2f(0, player.velocity.y < 0 ? -player.distanceY : player.distanceY));
                        player.velocity.y *= -1;
                    }
                    if (player.side == CollisionSide::Left or player.side == CollisionSide::Right) {
                        player.sprite.move(sf::Vector2f(player.velocity.x < 0 ? -player.distanceX : player.distanceX, 0));
                        player.velocity.x *= -1;
                    }
                }
            }
            for (auto& projectile: projectileVec) {
                //DEBUGGER -> WALL
                if (auto* debuggerProjectile = dynamic_cast<DebuggerProjectile*>(projectile.get()); debuggerProjectile and debuggerProjectile->checkCollision(*wall)) {
                    if (!debuggerProjectile->upgraded) {
                        view.shake(5);
                        debuggerProjectile->alive = false;
                        effectVec.emplace_back(std::make_unique<Effect>(Effect(debuggerProjectile->sprite.getPosition(), effectTexMap.at("Miss"))));
                    }
                }
                //COLLISION TRIGGER -> WALL
                if (auto* collisionTriggerProjectile = dynamic_cast<CollisionTriggerProjectile*>(projectile.get()); collisionTriggerProjectile and collisionTriggerProjectile->checkCollision(*wall)) {
                    if (collisionTriggerProjectile->upgraded) {
                        collisionTriggerProjectile->lifeTime = 0;
                        if (collisionTriggerProjectile->side == CollisionSide::Top or collisionTriggerProjectile->side == CollisionSide::Bottom) {
                            collisionTriggerProjectile->sprite.move(sf::Vector2f(0, collisionTriggerProjectile->direction.y < 0 ? -collisionTriggerProjectile->distanceY : collisionTriggerProjectile->distanceY));
                            collisionTriggerProjectile->direction.y *= -1;
                        }
                        if (collisionTriggerProjectile->side == CollisionSide::Left or collisionTriggerProjectile->side == CollisionSide::Right) {
                            collisionTriggerProjectile->sprite.move(sf::Vector2f(collisionTriggerProjectile->direction.x < 0 ? -collisionTriggerProjectile->distanceX : collisionTriggerProjectile->distanceX, 0));
                            collisionTriggerProjectile->direction.x *= -1;
                        }
                        if (!collisionTriggerProjectile->bounced) {
                            view.shake(10);
                            collisionTriggerProjectile->bounced = true;
                        }
                        else
                            view.shake(5);
                    }
                    else {
                        view.shake(5);
                        collisionTriggerProjectile->alive = false;
                        effectVec.emplace_back(std::make_unique<Effect>(Effect(collisionTriggerProjectile->sprite.getPosition(), effectTexMap.at("Miss"))));
                    }
                }
                //THREAD MANAGER -> WALL
                if (auto* threadManagerProjectile = dynamic_cast<ThreadManagerProjectile*>(projectile.get()); threadManagerProjectile and threadManagerProjectile->checkCollision(*wall)) {
                    view.shake(5);
                    threadManagerProjectile->alive = false;
                    effectVec.emplace_back(std::make_unique<Effect>(Effect(threadManagerProjectile->sprite.getPosition(), effectTexMap.at("Miss"))));
                }
            }
            //SYNTAX ERROR -> WALL
            if (!player.performanceMode) {
                for (auto& enemy: enemyVec) {
                    if (auto* syntaxError = dynamic_cast<SyntaxError*>(enemy.get()); syntaxError and syntaxError->tile == player.tile and syntaxError->checkCollision(*wall)) {
                        if (syntaxError->side == CollisionSide::Top or syntaxError->side == CollisionSide::Bottom) {
                            syntaxError->sprite.move(sf::Vector2f(0, syntaxError->velocity.y < 0 ? -syntaxError->distanceY : syntaxError->distanceY));
                            syntaxError->velocity.y *= -1;
                        }
                        if (syntaxError->side == CollisionSide::Left or syntaxError->side == CollisionSide::Right) {
                            syntaxError->sprite.move(sf::Vector2f(syntaxError->velocity.x < 0 ? -syntaxError->distanceX : syntaxError->distanceX, 0));
                            syntaxError->velocity.x *= -1;
                        }
                    }
                }
            }
        }
        //WITH ENEMY
        for (auto& enemy: enemyVec) {
            //PLAYER -> ENEMY
            if (player.checkCollision(*enemy)) {
                if (auto* error = dynamic_cast<Error*>(enemy.get()); error) {
                    error->alive = false;
                    player.increaseKnowledge(.1f);
                    effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("Knowledge"))));
                    effectVec.emplace_back(std::make_unique<Effect>(Effect(error->sprite.getPosition(), effectTexMap.at("Compiled"))));
                }
                switch (player.side) {
                    case CollisionSide::Top: player.velocity.y = -10; break;
                    case CollisionSide::Bottom: player.velocity.y = 10; break;
                    case CollisionSide::Left: player.velocity.x = -10; break;
                    case CollisionSide::Right: player.velocity.x = 10; break;
                }
                view.shake(10);
                player.health -= enemy->getDamage();
            }

            for (auto& projectile : projectileVec) {
                //EXCEPTION -> PLAYER
                if (auto* exceptionThrowerProjectile = dynamic_cast<ExceptionThrowerProjectile*>(projectile.get()); exceptionThrowerProjectile and player.checkCollision(*exceptionThrowerProjectile) and exceptionThrowerProjectile->alive) {
                    view.shake(15);
                    exceptionThrowerProjectile->alive = false;
                    player.health -= exceptionThrowerProjectile->damage;
                }
                //DEBUGGER
                if (auto* debuggerProjectile = dynamic_cast<DebuggerProjectile*>(projectile.get()); debuggerProjectile) {
                    //DEBUGGER -> SYNTAX ERROR
                    if (auto* syntaxError = dynamic_cast<SyntaxError*>(enemy.get()); syntaxError and debuggerProjectile->checkCollision(*syntaxError) and !syntaxError->compile) {
                        view.shake(5);
                        debuggerProjectile->alive = false;
                        if (syntaxError->requiredSide == debuggerProjectile->side and !syntaxError->compile) {
                            syntaxError->animationSpeed = 0;
                            syntaxError->animationFrame = 0;
                            syntaxError->compile = true;
                        }
                        else
                            effectVec.emplace_back(std::make_unique<Effect>(Effect(debuggerProjectile->sprite.getPosition(), effectTexMap.at("Miss"))));
                    }
                    //DEBUGGER -> EXCEPTION THROWER
                    if (auto* exceptionThrower = dynamic_cast<ExceptionThrower*>(enemy.get()); exceptionThrower and debuggerProjectile->checkCollision(*exceptionThrower) and !exceptionThrower->compile) {
                        view.shake(5);
                        debuggerProjectile->alive = false;
                        exceptionThrower->animationSpeed = 0;
                        exceptionThrower->animationFrame = 0;
                        exceptionThrower->compile = true;
                    }
                    //DEBUGGER -> ERROR
                    if (auto* error = dynamic_cast<Error*>(enemy.get()); error and debuggerProjectile->checkCollision(*error)) {
                        view.shake(5);
                        player.increaseKnowledge(.15f);
                        effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("Knowledge"))));
                        effectVec.emplace_back(std::make_unique<Effect>(Effect(error->sprite.getPosition(), effectTexMap.at("Compiled"))));
                        debuggerProjectile->alive = false;
                        error->alive = false;
                    }
                }

                //COLLISION TRIGGER
                if (auto* collisionTriggerProjectile = dynamic_cast<CollisionTriggerProjectile*>(projectile.get())) {
                    //COLLISION TRIGGER -> SYNTAX ERROR
                    if (auto* syntaxError = dynamic_cast<SyntaxError*>(enemy.get()); syntaxError and collisionTriggerProjectile->checkCollision(*syntaxError)) {
                        view.shake(5);
                        collisionTriggerProjectile->alive = false;
                        if (syntaxError->requiredSide == projectile->side and !syntaxError->compile) {
                            syntaxError->animationSpeed = 0;
                            syntaxError->animationFrame = 0;
                            syntaxError->compile = true;
                        }
                        else
                            effectVec.emplace_back(std::make_unique<Effect>(Effect(collisionTriggerProjectile->sprite.getPosition(), effectTexMap.at("Miss"))));
                    }
                    //COLLISION TRIGGER -> EXCEPTION THROWER
                    if (auto* exceptionThrower = dynamic_cast<ExceptionThrower*>(enemy.get()); exceptionThrower and collisionTriggerProjectile->checkCollision(*exceptionThrower) and !exceptionThrower->compile) {
                        view.shake(5);
                        collisionTriggerProjectile->alive = false;
                        exceptionThrower->animationSpeed = 0;
                        exceptionThrower->animationFrame = 0;
                        exceptionThrower->compile = true;
                    }
                    //COLLISION TRIGGER -> ERROR
                    if (auto* error = dynamic_cast<Error*>(enemy.get()); error and collisionTriggerProjectile->checkCollision(*error)) {
                        view.shake(5);
                        collisionTriggerProjectile->alive = false;
                        error->alive = false;
                        player.increaseKnowledge(.15f);
                        effectVec.emplace_back(std::make_unique<Effect>(Effect(player.sprite.getPosition(), effectTexMap.at("Knowledge"))));
                        effectVec.emplace_back(std::make_unique<Effect>(Effect(error->sprite.getPosition(), effectTexMap.at("Compiled"))));
                    }
                }
                //THREAD MANAGER
                if (auto* threadManagerProjectile = dynamic_cast<ThreadManagerProjectile*>(projectile.get())) {
                    //THREAD MANAGER -> SYNTAX ERROR
                    if (auto* syntaxError = dynamic_cast<SyntaxError*>(enemy.get()); syntaxError and threadManagerProjectile->checkCollision(*syntaxError) and !syntaxError->compile) {
                        view.shake(15);
                        threadManagerProjectile->alive = false;
                        enemy->sleepLength = 20;
                        effectVec.emplace_back(std::make_unique<Effect>(Effect(threadManagerProjectile->sprite.getPosition(), effectTexMap.at("Sleep"))));
                    }
                    //THREAD MANAGER -> EXCEPTION THROWER
                    if (auto* exceptionThrower = dynamic_cast<ExceptionThrower*>(enemy.get()); exceptionThrower and threadManagerProjectile->checkCollision(*exceptionThrower) and !exceptionThrower->compile) {
                        view.shake(15);
                        threadManagerProjectile->alive = false;
                        enemy->sleepLength = 20;
                        effectVec.emplace_back(std::make_unique<Effect>(Effect(threadManagerProjectile->sprite.getPosition(), effectTexMap.at("Sleep"))));
                    }
                }
            }
        }

        //WITH KNOWLEDGE
        for (auto& knowledge: knowledgeVec) {
            for (auto& projectile: projectileVec)
                if (knowledge->checkCollision(*projectile)) {
                    knowledge->alive = false;
                    effectVec.emplace_back(std::make_unique<Effect>(Effect(knowledge->sprite.getPosition(), effectTexMap.at("Miss"))));
                }
        }

        //GAME OVER?
        if (player.health <= 0)
            menu.mainMenu = true;

        //UPDATE ENTITIES
        if (!menu.pauseMenu) {
            for (auto& weapon : weaponVec)
                weapon->update(timeScale, player);
            for (auto& projectile : projectileVec)
                projectile->update(timeScale);
            for (auto& enemy : enemyVec) {
                if (enemy->tile == player.tile)
                    enemy->update(timeScale, player);
            }
            for (auto& knowledge : knowledgeVec) {
                if (knowledge->tile == player.tile)
                    knowledge->update(timeScale, player);
            }
        }
        for (auto& effect : effectVec)
            effect->update(timeScale);

        //REMOVE UNWANTED ENTITIES
        auto it1 = std::ranges::remove_if(projectileVec, [](const std::unique_ptr<Projectile>& projectile) {return !projectile->alive;});
        projectileVec.erase(it1.begin(), projectileVec.end());
        auto it3 = std::ranges::remove_if(effectVec, [](const std::unique_ptr<Effect>& effect) {return !effect->alive;});
        effectVec.erase(it3.begin(), effectVec.end());
        auto it4 = std::ranges::remove_if(enemyVec, [](const std::unique_ptr<Enemy>& enemy) {return !enemy->alive;});
        enemyVec.erase(it4.begin(), enemyVec.end());
        auto it5 = std::ranges::remove_if(weaponVec, [](const std::unique_ptr<Weapon>& weapon) {return !weapon->alive;});
        weaponVec.erase(it5.begin(), weaponVec.end());
        auto it6 = std::ranges::remove_if(knowledgeVec, [](const std::unique_ptr<Knowledge>& knowledge) {return !knowledge->alive;});
        knowledgeVec.erase(it6.begin(), knowledgeVec.end());

        //CLEAR
        window.clear(sf::Color::Black);

        //DRAW BACKGROUND TILES
        for (int i = 0; i < tilePositionVec.size(); ++i)
            if (tileCleared.at(i))
                window.draw(tileSpriteMap.at("FloorCompiled").at(i));
            else
                window.draw(tileSpriteMap.at("FloorNormal").at(i));

        //DRAW ENTITIES
        for (auto& effect : effectVec)
            if (effect->type == 0 and !player.performanceMode)
                window.draw(effect->sprite);
        for (auto& knowledge : knowledgeVec)
            window.draw(knowledge->sprite);
        for (auto& weapon : weaponVec)
            window.draw(weapon->sprite);
        for (auto& enemy : enemyVec)
            window.draw(enemy->sprite);

        window.draw(player.shadowSprite);
        window.draw(player.sprite);

        //DRAW FOREGROUND TILES
        window.draw(backgroundSprite);
        for (int i = 0; i < tilePositionVec.size(); ++i) {
            if (tileCleared.at(i))
                window.draw(tileSpriteMap.at("WallCompiled").at(i));
            else
                window.draw(tileSpriteMap.at("WallNormal").at(i));
        }

        for (auto& effect : effectVec)
            if (effect->type == 1 and !player.performanceMode)
                window.draw(effect->sprite);

        for (auto& projectile : projectileVec)
            window.draw(projectile->sprite);

        for (int i = 0; i < tilePositionVec.size(); ++i) {
            if (tileCleared.at(i))
                window.draw(tileCoverSpriteMap.at("CoverCompiled").at(i).sprite);
            else
                window.draw(tileCoverSpriteMap.at("CoverNormal").at(i).sprite);
        }

        if (menu.pauseMenu) {
            menu.update(view);
            for (auto& pauseMenuElement : menu.pauseMenuVec)
                window.draw(*pauseMenuElement);
        }

        //DRAW OVERLAYS
        if (view.showOverlay) {
            window.draw(player.healthBarBackground);
            window.draw(player.healthBar);
            window.draw(player.healthBarOutline);
            window.draw(player.staminaBarBackground);
            window.draw(player.staminaBar);
            window.draw(player.staminaBarOutline);
            window.draw(player.knowledgeBarBackground);
            window.draw(player.knowledgeBar);
            window.draw(player.healthIcon);
            window.draw(player.staminaIcon);
            window.draw(player.inventoryIcon);
            window.draw(player.knowledgeIcon);
            window.draw(player.knowledgeLevelSprite);
            if (player.upgradePoints > 0 and !(player.healthLevel >= 3 and player.staminaLevel >= 3 and player.inventoryCapacity >= 3)) {
                window.draw(player.unspentPoints);
                window.draw(player.healthSprite);
                window.draw(player.staminaSprite);
                window.draw(player.slotSprite);
            }
            for (int i = 0; i < player.inventoryCapacity; ++i)
                window.draw(player.inventorySlots.at(i));
            for (auto& weapon : player.inventory)
                window.draw(weapon->sprite);
            if (std::ranges::find_if(enemyVec, [&](const auto& enemy){ return enemy->tile == player.tile;}) == enemyVec.end() and !tileCleared.at(player.tile - 1))
                window.draw(player.compileButton);
            if (std::ranges::all_of(tileCleared.begin(), tileCleared.end(), [](int x) {return x == 1;}))
                window.draw(player.winButton);
        }

        //DRAW WALL COLLISION (ONLY DEBUGGING)
        /*for (auto& wall : wallVec)
            window.draw(wall->sprite);*/

        if (player.showFPS) {
            fps.setPosition(sf::Vector2f(view.view.getCenter().x + 860, view.view.getCenter().y - 490));
            fps.setString(std::to_string(static_cast<int>(60 / timeScale)));
            window.draw(fps);
        }

        //DISPLAY
        window.display();

    }
}