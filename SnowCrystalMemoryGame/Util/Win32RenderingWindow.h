#pragma once

#include <windows.h>
#include <string>

#include "../Rendering/D3D11Renderer.h"

class Win32RenderingWindow {
public:
    Win32RenderingWindow(std::string applicationName, bool fullscreenEnabled, int screenWidth, int screenHeight, HINSTANCE& hInstance);
    ~Win32RenderingWindow();

    void showWindow();
    HWND getWindowHandle() const;
    void pollForMessage(MSG* msg);
    void postQuitMessage();

private:
    HWND windowHandle;

    static LRESULT CALLBACK WindowProc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT handleWindowMsg(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);
};

class Win32RenderingWindowException : public std::exception {
private:
    DWORD errorCode;
    const char* msg;

public:
    Win32RenderingWindowException(const char* msg) : errorCode(GetLastError()), msg(msg) {
    }

    DWORD getErrorCode() const { return errorCode; }

    const char* what() const noexcept override {
        return msg;
    }
};
