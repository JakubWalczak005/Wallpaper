#include <SFML/Graphics.hpp>
#include <windows.h>
#include <shellapi.h>
#include <fmt/base.h>
#include <shlobj.h>

struct WindowData {
    sf::RenderWindow* window;
    HWND progman;
};

void addTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    lstrcpy(nid.szTip, TEXT("App"));
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void removeTrayIcon(HWND hwnd) {
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

/*void cleanup(HWND hwnd, HWND originalParent) {
    removeTrayIcon(hwnd);
    SetParent(hwnd, originalParent);
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    style = (style & ~WS_CHILD) | WS_POPUP;
    SetWindowLong(hwnd, GWL_STYLE, style);
    ShowWindow(hwnd, SW_HIDE);
    UpdateWindow(hwnd);
    SendMessageTimeout(originalParent, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    SendMessageTimeout(originalParent, WM_PAINT, 0, 0, SMTO_NORMAL, 1000, nullptr);
    SendMessageTimeout(originalParent, WM_PAINT, 0, 0, SMTO_NORMAL, 1000, nullptr);
    InvalidateRect(originalParent, nullptr, TRUE);
    UpdateWindow(originalParent);
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
}*/

LRESULT CALLBACK customWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
        case WM_APP + 1:
            switch (lParam) {
                case WM_RBUTTONUP: {
                    POINT pt;
                    GetCursorPos(&pt);

                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, 1001, TEXT("Exit"));

                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
                    DestroyMenu(hMenu);
                    break;
                }
            }
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                auto data = reinterpret_cast<WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                data->window->close();
                //cleanup(hwnd, data->progman);
                PostQuitMessage(0);
            }
        break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

auto main() -> int {

    auto window = sf::RenderWindow(sf::VideoMode({2560, 1440}), "Wallpaper", sf::Style::None, sf::State::Windowed, sf::ContextSettings{.antiAliasingLevel = 8});

    HWND hwnd = window.getNativeHandle();
    WNDPROC g_originalWndProc = nullptr;
    g_originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(customWndProc)));
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
    addTrayIcon(hwnd);

    HWND progman = FindWindow(TEXT("Progman"), nullptr);
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);

    static WindowData windowData;
    windowData.window = &window;
    windowData.progman = FindWindow(TEXT("Progman"), nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&windowData));

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
        SetParent(hwnd, progman);
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE, (style & ~WS_POPUP) | WS_CHILD);
    }

    if (workerw) {
        fmt::print("Found WorkerW: {}\n", (void*)workerw);
    }
    else {
        fmt::print("No suitable WorkerW found!\n");
    }

    MSG msg = {};

    while (window.isOpen()) {

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        window.clear(sf::Color::Red);
        window.display();
    }

}