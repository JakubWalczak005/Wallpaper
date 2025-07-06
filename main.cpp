#include <SFML/Graphics.hpp>
#include <windows.h>
#include <shellapi.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <shlobj.h>
#include "Folder.h"
#include "DropTarget.h"
#include <Ole2.h>

struct WindowData {
    sf::RenderWindow* window;
};

void addTrayIcon(HWND trayHwnd);
void removeTrayIcon(HWND trayHwnd);
LRESULT CALLBACK customWndProc(HWND trayHwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool IsWindowAbove(HWND hwnd1, HWND hwnd2);
void leftClick(const sf::RenderWindow& window);

auto folderVec = std::vector<std::unique_ptr<Custom::Folder>>();

auto main() -> int {

    //TEXTURES
    auto placeholder100 = sf::Texture("..\\Files\\Placeholders\\placeholder100.png");

    //WINDOW
    auto window = sf::RenderWindow(sf::VideoMode({7680, 1440}), "Wallpaper", sf::Style::None, sf::State::Windowed, sf::ContextSettings{.antiAliasingLevel = 8});
    window.setFramerateLimit(60);
    HWND hwnd = window.getNativeHandle();
    SetWindowLongPtr(hwnd, GWL_STYLE, (GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_POPUP) | WS_CLIPCHILDREN);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_ACCEPTFILES);
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    fmt::println("{}", CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
    auto dropTarget = new DropTarget(hwnd);
    fmt::print("{}", RegisterDragDrop(hwnd, dropTarget));

    /*HWND progman = FindWindow(TEXT("Progman"), nullptr);
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    HWND workerw = nullptr;
    EnumWindows([](HWND topHandle, LPARAM lParam) -> BOOL {
        HWND shellView = FindWindowEx(topHandle, nullptr, TEXT("SHELLDLL_DefView"), nullptr);
        if (shellView != nullptr) {
            HWND* out = reinterpret_cast<HWND*>(lParam);
            *out = FindWindowEx(nullptr, topHandle, TEXT("WorkerW"), nullptr);
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&workerw));
    if (workerw != nullptr) {
        SetWindowLong(hwnd, GWL_EXSTYLE, (GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_POPUP) | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
        SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }*/

    //TRANSPARENT INPUT CATCHER
    /*auto inputCatcher = sf::RenderWindow(sf::VideoMode({7680, 1440}), "Input Catcher", sf::Style::None, sf::State::Windowed);
    HWND inputHwnd = inputCatcher.getNativeHandle();
    SetWindowLong(inputHwnd, GWL_EXSTYLE, (GetWindowLong(inputHwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE));
    SetWindowPos(inputHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);*/

    //TRANSPARENT TRAY WINDOW
    WNDCLASS wc = {};
    wc.lpfnWndProc = customWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = TEXT("Tray");
    RegisterClass(&wc);
    HWND trayHwnd = CreateWindowEx(0, TEXT("Tray"), nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, GetModuleHandle(nullptr), nullptr);
    addTrayIcon(trayHwnd);
    static WindowData windowData;
    windowData.window = &window;
    SetWindowLongPtr(trayHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&windowData));

    //UTILITY
    MSG msg = {};
    auto font = sf::Font("..\\Files\\Font.TTF");
    auto text = sf::Text(font);
    text.setPosition({100, 100});
    text.setString("hello world");

    //SPAWN
    folderVec.push_back(std::make_unique<Custom::Folder>(Custom::Folder({1000, 1000}, placeholder100, std::string("D:"))));

    while (window.isOpen()) {

        HWND taskbarHwnd = FindWindow(TEXT("Shell_TrayWnd"), nullptr);
        if (IsWindowAbove(hwnd, taskbarHwnd))
            SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        while (auto event = window.pollEvent()) {

            if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                switch (mouseClickEvent->button) {
                    case sf::Mouse::Button::Left: {
                        auto mousePos = sf::Mouse::getPosition(window);
                        text.setString(fmt::format("{} {}", mousePos.x, mousePos.y));
                        leftClick(window);
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



        window.clear(sf::Color::Black);
        window.draw(text);
        for (auto& folder : folderVec)
            window.draw(folder->sprite);
        window.display();

    }

    dropTarget->Release();
    RevokeDragDrop(hwnd);
    CoUninitialize();

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

LRESULT CALLBACK customWndProc(HWND trayHwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto windowData = reinterpret_cast<WindowData*>(GetWindowLongPtr(trayHwnd, GWLP_USERDATA));
    switch (msg) {
        case WM_APP + 1:
            switch (lParam) {
                case WM_RBUTTONUP: {
                    POINT pt;
                    GetCursorPos(&pt);
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, 1001, TEXT("Exit"));
                    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, trayHwnd, nullptr);
                    DestroyMenu(hMenu);
                }
            }
        break;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                windowData->window->clear(sf::Color::Black);
                windowData->window->display();
                windowData->window->close();
                removeTrayIcon(trayHwnd);
                PostQuitMessage(0);
            }
        break;
    }
    return DefWindowProc(trayHwnd, msg, wParam, lParam);
}

bool IsWindowAbove(HWND hwnd1, HWND hwnd2) {
    HWND hwnd = GetTopWindow(nullptr);
    while (hwnd) {
        if (hwnd == hwnd1) return true;
        if (hwnd == hwnd2) return false;
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return false;
}

void leftClick(const sf::RenderWindow& window) {
    for (auto& folder : folderVec)
        if (folder->customBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window))))
            ShellExecute(nullptr, TEXT("open"), folder->path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void createWindow() {
    auto window = sf::RenderWindow(sf::VideoMode({1000, 1000}), "Wallpaper", sf::Style::None, sf::State::Windowed, sf::ContextSettings{.antiAliasingLevel = 8});

}