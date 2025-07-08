#include <SFML/Graphics.hpp>
#include <windows.h>
#include <shellapi.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <shlobj.h>
#include "Folder.h"
#include "DropTarget.h"

struct WindowData {
    sf::RenderWindow* window; //unused
    bool run = true;
};

HWND findWorkerW();
void addTrayIcon(HWND trayHwnd);
void removeTrayIcon(HWND trayHwnd);
LRESULT CALLBACK trayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool isWindowAbove(HWND hwnd1, HWND hwnd2);
void leftClick(const sf::RenderWindow& window, std::wstring& newWindowPath);

auto windowVec = std::vector<std::unique_ptr<sf::RenderWindow>>();
auto folderVec = std::vector<std::unique_ptr<Custom::Folder>>();

constexpr auto WIDTH = 7680;
constexpr auto HEIGHT = 1440;

auto main() -> int {

    //TEXTURES
    auto placeholder100 = sf::Texture("..\\Files\\Placeholders\\placeholder100.png");

    //WINDOW
    HWND workerW = findWorkerW();
    auto mainWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode({WIDTH, HEIGHT}), "Wallpaper", sf::Style::None, sf::State::Windowed);
    HWND mainHwnd = mainWindow->getNativeHandle();
    SetWindowLongPtr(mainHwnd, GWL_STYLE, (GetWindowLongPtr(mainHwnd, GWL_STYLE) & ~WS_POPUP) | WS_CHILD);
    SetWindowLongPtr(mainHwnd, GWL_EXSTYLE, GetWindowLongPtr(mainHwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
    SetParent(mainHwnd, workerW);
    SetWindowPos(mainHwnd, nullptr, 0, 0, WIDTH, HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    fmt::println("mainWindow parent HWND: {}", (void*)GetParent(mainHwnd));
    fmt::println("workerW HWND: {}", (void*)workerW);
    auto renderTexture = sf::RenderTexture({WIDTH, HEIGHT}); //potential problem with antialiasing

    //TRANSPARENT INPUT CATCHER
    auto inputCatcherWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode({WIDTH, HEIGHT}), "Input Catcher", sf::Style::None, sf::State::Windowed);
    HWND inputHwnd = inputCatcherWindow->getNativeHandle();
    SetWindowLong(inputHwnd, GWL_EXSTYLE, (GetWindowLong(inputHwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE));
    SetWindowPos(inputHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    fmt::println("{}", OleInitialize(nullptr));
    auto dropTarget = new DropTarget(inputHwnd);
    fmt::print("{}", RegisterDragDrop(inputHwnd, dropTarget));
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
    auto fps = sf::Text(font);
    fps.setPosition({500, 100});
    auto deltaTime = sf::Clock();
    auto timeScale = 0.f;

    //SPAWN
    auto newWindowPath = std::wstring();
    //windowVec.push_back(std::move(mainWindow));
    windowVec.push_back(std::move(inputCatcherWindow));
    folderVec.push_back(std::make_unique<Custom::Folder>(sf::Vector2f(1000, 1000), placeholder100, std::string("D:")));

    while (windowData.run) {

        timeScale = 60 * deltaTime.restart().asSeconds();

        /*auto file = std::ofstream("..\\Files\\Save\\Save.txt");
        file.close();
        file = std::ofstream("..\\Files\\Save\\Save.txt", std::ios::app);
        if (file)
            for (auto& saveElement : saveState)
                file << saveElement << " ";*/

        /*HWND taskbarHwnd = FindWindow(TEXT("Shell_TrayWnd"), nullptr);
        if (isWindowAbove(mainHwnd, taskbarHwnd))
            SetWindowPos(mainHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);*/

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        for (auto& window : windowVec) {
            while (auto event = window->pollEvent()) {

                if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    switch (mouseClickEvent->button) {
                        case sf::Mouse::Button::Left: {
                            auto mousePos = sf::Mouse::getPosition(*window);
                            text.setString(fmt::format("{} {}", mousePos.x, mousePos.y));
                            leftClick(*window, newWindowPath);
                            //SetWindowPos(mainHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                            //SetWindowPos(inputHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                            break;
                        }
                        case sf::Mouse::Button::Right: {
                        }
                    }
                }
                if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                    switch (keyEvent->code) {
                        case sf::Keyboard::Key::W: fmt::print("works");
                    }
                }

            }
        }

        //renderTexture.clear(sf::Color::Black);
        for (auto& window : windowVec) {
            window->clear();
            window->draw(text);
            window->draw(fps);
            for (auto& folder : folderVec)
                window->draw(folder->sprite);
            window->display();
        }

        /*mainWindow->clear(sf::Color::Black);
        sf::Sprite sprite(renderTexture.getTexture());
        mainWindow->draw(sprite);
        mainWindow->display();*/

        if (!newWindowPath.empty()) {
            windowVec.push_back(std::make_unique<sf::RenderWindow>(sf::VideoMode({1000, 1000}), "Folder", sf::Style::Default, sf::State::Windowed));
            //SetWindowLongPtr(windowVec.back()->getNativeHandle(), GWL_EXSTYLE, GetWindowLongPtr(windowVec.back()->getNativeHandle(), GWL_EXSTYLE) | WS_EX_NOACTIVATE);
            newWindowPath.clear();
        }

        fps.setString(std::to_string(static_cast<int>(60 / timeScale)));

    }

    RevokeDragDrop(mainHwnd);
    OleUninitialize();
    for (auto& window : windowVec)
        window->close();

}

HWND findWorkerW() {
    HWND progman = FindWindow(TEXT("Progman"), nullptr);
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    HWND workerW = nullptr;
    EnumWindows([](HWND topHandle, LPARAM lParam) -> BOOL {
        HWND shellView = FindWindowEx(topHandle, nullptr, TEXT("SHELLDLL_DefView"), nullptr);
        if (shellView != nullptr) {
            HWND* out = reinterpret_cast<HWND*>(lParam);
            *out = FindWindowEx(nullptr, topHandle, TEXT("WorkerW"), nullptr);
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&workerW));
    return workerW;
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

LRESULT CALLBACK trayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto windowData = reinterpret_cast<WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (msg) {
        case WM_APP + 1:
            switch (lParam) {
                case WM_RBUTTONUP: {
                    POINT pt;
                    GetCursorPos(&pt);
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, 1001, TEXT("Exit"));
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

void leftClick(const sf::RenderWindow& window, std::wstring& newWindowPath) {
    for (auto& folder : folderVec)
        if (folder->customBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) {
            newWindowPath = folder->path;
        }
            //ShellExecute(nullptr, TEXT("open"), folder->path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}