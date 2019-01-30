#include "Win32RenderingWindow.h"

#include <stdexcept>

Win32RenderingWindow::Win32RenderingWindow(std::string applicationName, bool fullscreenEnabled, int screenWidth, int screenHeight, HINSTANCE& hInstance) {
    WNDCLASS windowClass;
    windowClass.lpszClassName = L"Rendering Window Class";
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    windowClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.lpszMenuName = nullptr;

    RegisterClass(&windowClass);

    int positionX, positionY;
    if (fullscreenEnabled) {
        DEVMODE dmScreenSettings;
        // If full screen set the screen to maximum size of the users desktop and 32bit.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = static_cast<unsigned long>(screenWidth);
        dmScreenSettings.dmPelsHeight = static_cast<unsigned long>(screenHeight);
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Change the display settings to full screen.
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        // Set the position of the window to the top left corner.
        positionX = positionY = 0;
    } else {
        positionX = CW_USEDEFAULT;
        positionY = CW_USEDEFAULT;
    }

    windowHandle = CreateWindowEx(
        0, // Optional window styles.
        windowClass.lpszClassName, // Window class
        std::wstring(applicationName.begin(), applicationName.end()).c_str(), // Window text
        WS_OVERLAPPEDWINDOW, // Window style

        // Size and position
        positionX, positionY,
        screenWidth, screenHeight,

        nullptr, // Parent window
        nullptr, // Menu
        hInstance, // Instance handle
        this // Additional application data
    );

    if (windowHandle == nullptr) {
        throw Win32RenderingWindowException("failed to create window");
    }
}

Win32RenderingWindow::~Win32RenderingWindow() {
    ChangeDisplaySettings(nullptr, 0);
}

void Win32RenderingWindow::showWindow() {
    ShowWindow(windowHandle, SW_SHOW);
    SetForegroundWindow(windowHandle);
    SetFocus(windowHandle);
}

HWND Win32RenderingWindow::getWindowHandle() const {
    return windowHandle;
}

void Win32RenderingWindow::pollForMessage(MSG* msg) {
    if (PeekMessage(msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(msg);
        DispatchMessage(msg);
    }
}

void Win32RenderingWindow::postQuitMessage() {
    PostQuitMessage(0);
}

LRESULT Win32RenderingWindow::handleWindowMsg(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(windowHandle, msg, wParam, lParam);
    }
}

LRESULT Win32RenderingWindow::WindowProc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam) {
    Win32RenderingWindow* win32GraphicsApp = nullptr;

    if (msg == WM_NCCREATE) {
        auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        win32GraphicsApp = static_cast<Win32RenderingWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(win32GraphicsApp));
    } else {
        win32GraphicsApp = reinterpret_cast<Win32RenderingWindow*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
    }

    if (win32GraphicsApp) {
        return win32GraphicsApp->handleWindowMsg(windowHandle, msg, wParam, lParam);
    } else {
        return DefWindowProc(windowHandle, msg, wParam, lParam);
    }
}
