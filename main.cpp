#include <SFML/Graphics.hpp>
#include <windows.h>
#include <shellapi.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <shlobj.h>
#include "FileExplorer/Folder.h"
#include "DropTarget.h"
#include "FileExplorerManager.h"
#include "FileExplorer/FileExplorer.h"
#include <regex>
#include <thread>

struct WindowData {
    bool run = true;
};

void addTrayIcon(HWND trayHwnd);
void removeTrayIcon(HWND trayHwnd);
LRESULT CALLBACK mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK trayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool isWindowAbove(HWND hwnd1, HWND hwnd2);

constexpr auto WIDTH = 7680;
constexpr auto HEIGHT = 1440;

WNDPROC defaultWndProc = nullptr;

/*void func(const std::filesystem::path& path) {
    auto tex = sf::Texture();
    auto clock = sf::Clock();
    vec.emplace_back(path);
    fmt::println("{}", clock.getElapsedTime().asSeconds());
    std::this_thread::sleep_for(std::chrono::seconds(10));
}
for (const auto& entry : std::filesystem::directory_iterator("D:\\ProgrammingProjects\\Wallpaper\\Files\\8k")) {
    std::thread t(func, entry.path());
    t.detach();
}*/

auto main() -> int {

    //TEXTURES
    auto placeholder100 = sf::Texture("..\\Files\\Placeholders\\placeholder100.png");

    //MAIN WINDOW
    auto mainWindow = sf::RenderWindow(sf::VideoMode({WIDTH, HEIGHT}), "Wallpaper", sf::Style::None, sf::State::Windowed);
    mainWindow.setFramerateLimit(240);
    HWND mainHwnd = mainWindow.getNativeHandle();
    defaultWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(mainHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(mainWndProc)));
    SetWindowLongPtr(mainHwnd, GWL_STYLE, (GetWindowLongPtr(mainHwnd, GWL_STYLE) & ~WS_POPUP));
    SetWindowLongPtr(mainHwnd, GWL_EXSTYLE, GetWindowLongPtr(mainHwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
    SetWindowPos(mainHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    OleInitialize(nullptr);
    auto dropTarget = new DropTarget(mainHwnd);
    RegisterDragDrop(mainHwnd, dropTarget);
    dropTarget->Release();

    //TRANSPARENT TRAY WINDOW
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = trayWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = TEXT("Tray");
        RegisterClass(&wc);
    }
    HWND trayHwnd = CreateWindowEx(0, TEXT("Tray"), nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, GetModuleHandle(nullptr), nullptr);
    addTrayIcon(trayHwnd);
    static WindowData windowData;
    SetWindowLongPtr(trayHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&windowData));

    //UTILITY
    MSG msg = {};
    auto font = sf::Font("..\\Files\\Font.TTF");
    auto text = sf::Text(font);
    text.setPosition({100, 100});
    text.setString("hello world");
    auto refresh = sf::Text(font);
    refresh.setPosition({300, 100});
    auto fps = sf::Text(font);
    fps.setPosition({500, 100});
    auto deltaTime = sf::Clock();
    auto timeScale = 0.f;
    auto refreshClock = sf::Clock();
    auto refreshTimer = 0.f;
    auto fileExplorerManager = FileExplorerManager();
    auto folderVec = std::vector<std::unique_ptr<Custom::Folder>>();

    //SPAWN
    folderVec.push_back(std::make_unique<Custom::Folder>(std::filesystem::path("D:\\test"), FileLayout::LargeIcons, sf::Vector2f(1000, 1000)));

    //MAIN LOOP
    while (windowData.run) {

        timeScale = 60 * deltaTime.restart().asSeconds();
        refreshTimer += refreshClock.restart().asSeconds();

        /*auto file = std::ofstream("..\\Files\\Save\\Save.txt");
        file.close();
        file = std::ofstream("..\\Files\\Save\\Save.txt", std::ios::app);
        if (file)
            for (auto& saveElement : saveState)
                file << saveElement << " ";*/

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        HWND taskbarHwnd = FindWindow(TEXT("Shell_TrayWnd"), nullptr);
        if (isWindowAbove(mainHwnd, taskbarHwnd))
            SetWindowPos(mainHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        while (auto event = mainWindow.pollEvent()) {

            if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                switch (mouseClickEvent->button) {
                    case sf::Mouse::Button::Left: {
                        for (auto& folder : folderVec)
                            if (folder->customBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(mainWindow))))
                                fileExplorerManager.fileExplorerVec.emplace_back(std::make_unique<FileExplorer>(FileExplorer(folder->absolutePath)));
                        auto mousePos = sf::Mouse::getPosition(mainWindow);
                        text.setString(fmt::format("{} {}", mousePos.x, mousePos.y));
                        break;
                    }
                    case sf::Mouse::Button::Right: {
                    }
                }
            }
            if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyEvent->code) {
                    case sf::Keyboard::Key::W: break;
                }
            }

        }

        //CLEAR
        mainWindow.clear(sf::Color::Black);

        //REFRESH INFO
        if (refreshTimer > .5f) {
            refreshTimer = 0;
            for (auto& fileExplorer : fileExplorerManager.fileExplorerVec)
                fileExplorer->refresh();
        }

        //DRAW SUBWINDOWS
        for (auto& folder : folderVec)
            folder->update(mainWindow);
        for (auto& fileExplorer : fileExplorerManager.fileExplorerVec)
            fileExplorer->update(fileExplorerManager);

        fileExplorerManager.updateFileExplorerVec();

        //DRAW MAINWINDOW
        fps.setString(std::to_string(static_cast<int>(60 / timeScale)));
        refresh.setString(std::to_string(refreshTimer));
        mainWindow.draw(text);
        mainWindow.draw(fps);
        mainWindow.draw(refresh);

        //DISPLAY
        mainWindow.display();

    }

    RevokeDragDrop(mainHwnd);
    OleUninitialize();
    mainWindow.close();
    for (auto& fileExplorer : fileExplorerManager.fileExplorerVec)
        fileExplorer->window.close();

}

void addTrayIcon(HWND trayHwnd) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = trayHwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    lstrcpy(nid.szTip, TEXT("Wallpaper"));
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void removeTrayIcon(HWND trayHwnd) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = trayHwnd;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

LRESULT CALLBACK mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_MOUSEACTIVATE)
        return MA_NOACTIVATE;
    return CallWindowProc(defaultWndProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK trayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto windowData = reinterpret_cast<WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (msg) {
        case WM_APP + 1:
            switch (lParam) {
                case WM_RBUTTONUP: {
                    POINT pt;
                    GetCursorPos(&pt);
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, 1001, TEXT("Quit"));
                    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
                    DestroyMenu(hMenu);
                }
            }
        break;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                removeTrayIcon(hwnd);
                PostQuitMessage(0);
                windowData->run = false;
            }
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool isWindowAbove(HWND hwnd1, HWND hwnd2) {
    HWND hwnd = GetTopWindow(nullptr);
    while (hwnd) {
        if (hwnd == hwnd1) return true;
        if (hwnd == hwnd2) return false;
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return false;
}