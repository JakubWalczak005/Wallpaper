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

void addTrayIcon(HWND trayHwnd);
void removeTrayIcon(HWND trayHwnd);
LRESULT CALLBACK trayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
HBITMAP CreateDIBFromSFML(const uint8_t* sfmlPixels, int width, int height);
bool isWindowAbove(HWND hwnd1, HWND hwnd2);
void leftClick(const HWND& hwnd, std::wstring& newWindowPath);

auto windowVec = std::vector<std::unique_ptr<sf::RenderWindow>>();
auto folderVec = std::vector<std::unique_ptr<Custom::Folder>>();

auto main() -> int {

    //TEXTURES
    auto placeholder100 = sf::Texture("..\\Files\\Placeholders\\placeholder100.png");

    /*fmt::println("{}", OleInitialize(nullptr));
    auto dropTarget = new DropTarget(hwnd);
    fmt::print("{}", RegisterDragDrop(hwnd, dropTarget));
    dropTarget->Release();*/

    HWND progman = FindWindow(TEXT("Progman"), nullptr);
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

    /*if (workerw != nullptr) {
        SetParent(hwnd, progman);
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE, (style & ~WS_POPUP) | WS_CHILD);
    }*/

    //WINDOW
    auto mainWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode({7680, 1440}), "Wallpaper", sf::Style::None, sf::State::Windowed);
    mainWindow->setFramerateLimit(60);
    HWND hwnd = mainWindow->getNativeHandle();
    SetWindowLongPtr(hwnd, GWL_STYLE, (GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_POPUP) | WS_CHILD);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
    SetWindowPos(hwnd, workerw, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetParent(hwnd, workerw);
    SetWindowPos(hwnd, nullptr, 0, 0, 7680, 1440, SWP_NOZORDER | SWP_SHOWWINDOW);

    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = mainWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = TEXT("Main");
        RegisterClass(&wc);
    }
    HWND mainHwnd = CreateWindowEx(0,TEXT("Main"), nullptr, WS_CHILD | WS_VISIBLE, 0, 0, 7680, 1440, workerw, nullptr, GetModuleHandle(nullptr),nullptr);
    auto renderTexture = sf::RenderTexture({7680, 1440}); //potential problem with antialiasing

    //TRANSPARENT INPUT CATCHER
    auto inputCatcher = sf::RenderWindow(sf::VideoMode({7680, 1440}), "Input Catcher", sf::Style::None, sf::State::Windowed);
    HWND inputHwnd = inputCatcher.getNativeHandle();
    SetWindowLong(inputHwnd, GWL_EXSTYLE, (GetWindowLong(inputHwnd, GWL_STYLE) | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE));
    SetWindowPos(inputHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

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
    fps.setPosition({200, 100});
    auto deltaTime = sf::Clock();
    auto timeScale = 0.f;

    //SPAWN
    auto newWindowPath = std::wstring();
    //windowVec.push_back(std::move(mainWindow));
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
                            leftClick(mainHwnd, newWindowPath);
                            //SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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

        for (auto& window : windowVec) {
            window->clear(sf::Color::Black);
            window->draw(text);
            for (auto& folder : folderVec)
                window->draw(folder->sprite);
            window->display();
        }

        if (!newWindowPath.empty()) {
            windowVec.push_back(std::make_unique<sf::RenderWindow>(sf::VideoMode({1000, 1000}), "Folder", sf::Style::Default, sf::State::Windowed, sf::ContextSettings{.antiAliasingLevel = 8}));
            SetClassLongPtr(windowVec.back()->getNativeHandle(), GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(GetStockObject(NULL_BRUSH)));
            SetWindowLongPtr(windowVec.back()->getNativeHandle(), GWL_EXSTYLE, GetWindowLongPtr(windowVec.back()->getNativeHandle(), GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
            SetWindowPos(windowVec.back()->getNativeHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
            newWindowPath.clear();
        }

        while (auto event = inputCatcher.pollEvent()) {

            if (auto mouseClickEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                switch (mouseClickEvent->button) {
                    case sf::Mouse::Button::Left: {
                        POINT mousePos;
                        GetCursorPos(&mousePos);
                        ScreenToClient(mainHwnd, &mousePos);
                        text.setString(fmt::format("{} {}", mousePos.x, mousePos.y));
                        leftClick(mainHwnd, newWindowPath);
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

        renderTexture.clear(sf::Color::Black);
        renderTexture.draw(text);
        for (auto& folder : folderVec)
            renderTexture.draw(folder->sprite);
        renderTexture.display();

        mainWindow->clear(sf::Color::Black);
        mainWindow->draw(fps);
        sf::Sprite sprite(renderTexture.getTexture());
        mainWindow->draw(sprite);
        mainWindow->display();

        /*auto image = renderTexture.getTexture().copyToImage();
        auto oldBmp = reinterpret_cast<HBITMAP>(GetWindowLongPtr(mainHwnd, GWLP_USERDATA));
        if (oldBmp)
            DeleteObject(oldBmp);
        SetWindowLongPtr(mainHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(CreateDIBFromSFML(image.getPixelsPtr(), image.getSize().x, image.getSize().y)));
        InvalidateRect(mainHwnd, nullptr, FALSE);*/

        fps.setString(std::to_string(static_cast<int>(60 / timeScale)));

    }


    RevokeDragDrop(mainHwnd);
    OleUninitialize();
    for (auto& window : windowVec)
        window->close();

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
                //windowData->run = false;
            }
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT:
            auto hBmp = reinterpret_cast<HBITMAP>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (hBmp) {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                HDC memDC = CreateCompatibleDC(hdc);
                HGDIOBJ old = SelectObject(memDC, hBmp);
                BitBlt(hdc, 0, 0, 7680, 1440, memDC, 0, 0, SRCCOPY);
                SelectObject(memDC, old);
                DeleteDC(memDC);
                EndPaint(hwnd, &ps);
            }
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HBITMAP CreateDIBFromSFML(const uint8_t* sfmlPixels, int width, int height) {
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;  // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);

    for (int i = 0; i < width * height; ++i) {
        static_cast<BYTE *>(pvBits)[i * 4 + 0] = sfmlPixels[i * 4 + 2];
        static_cast<BYTE *>(pvBits)[i * 4 + 1] = sfmlPixels[i * 4 + 1];
        static_cast<BYTE *>(pvBits)[i * 4 + 2] = sfmlPixels[i * 4 + 0];
        static_cast<BYTE *>(pvBits)[i * 4 + 3] = sfmlPixels[i * 4 + 3];
    }

    return hBitmap;
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

void leftClick(const HWND& hwnd, std::wstring& newWindowPath) {
    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(hwnd, &mousePos);
    for (auto& folder : folderVec)
        if (folder->customBounds().contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
            newWindowPath = folder->path;
        }
            //ShellExecute(nullptr, TEXT("open"), folder->path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}
