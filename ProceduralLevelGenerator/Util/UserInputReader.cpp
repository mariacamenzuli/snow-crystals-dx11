#include "UserInputReader.h"
#include <stdexcept>

UserInputReader::UserInputReader(HINSTANCE hinstance, HWND hwnd) {
    HRESULT result;

    result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(directInput.GetAddressOf()), nullptr);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create direct input.");
    }

    result = directInput->CreateDevice(GUID_SysKeyboard, keyboard.GetAddressOf(), nullptr);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create direct input keyboard.");
    }

    // Set the data format.  In this case since it is a keyboard we can use the predefined data format.
    result = keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to set they direct input keyboard data format.");
    }

    // Set the cooperative level of the keyboard to not share with other programs.
    result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(result)) {
        // no-op
    }

    // Now acquire the keyboard.
    result = keyboard->Acquire();
    if (FAILED(result)) {
        // no-op
    }

    // Initialize the direct input interface for the mouse.
    result = directInput->CreateDevice(GUID_SysMouse, mouse.GetAddressOf(), nullptr);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create direct input mouse.");
    }

    // Set the data format for the mouse using the pre-defined mouse data format.
    result = mouse->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create direct input mouse data format.");
    }

    // Set the cooperative level of the mouse to share with other programs.
    result = mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result)) {
        // no-op
    }

    // Acquire the mouse.
    result = mouse->Acquire();
    if (FAILED(result)) {
        // no-op
    }

    for (unsigned char& keyState : keyboardState) {
        keyState = '\0';
    }

    mouseState.lX = 0;
    mouseState.lY = 0;
    mouseState.lZ = 0;
}


UserInputReader::~UserInputReader() {
    mouse->Unacquire();
    keyboard->Unacquire();
}

void UserInputReader::read() {
    readKeyboard();
    readMouse();
}

bool UserInputReader::isEscapePressed() {
    return (keyboardState[DIK_ESCAPE] & 0x80) != 0;
}

bool UserInputReader::isQPressed() {
    return (keyboardState[DIK_Q] & 0x80) != 0;
}

bool UserInputReader::isWPressed() {
    return (keyboardState[DIK_W] & 0x80) != 0;
}

bool UserInputReader::isEPressed() {
    return (keyboardState[DIK_E] & 0x80) != 0;
}

bool UserInputReader::isAPressed() {
    return (keyboardState[DIK_A] & 0x80) != 0;
}

bool UserInputReader::isSPressed() {
    return (keyboardState[DIK_S] & 0x80) != 0;
}

bool UserInputReader::isDPressed() {
    return (keyboardState[DIK_D] & 0x80) != 0;
}

bool UserInputReader::isZPressed() {
    return (keyboardState[DIK_Z] & 0x80) != 0;
}

bool UserInputReader::isCPressed() {
    return (keyboardState[DIK_C] & 0x80) != 0;
}

bool UserInputReader::isRPressed() {
    return (keyboardState[DIK_R] & 0x80) != 0;
}

bool UserInputReader::isTPressed() {
    return (keyboardState[DIK_T] & 0x80) != 0;
}

bool UserInputReader::isSpacebarPressed() {
    return (keyboardState[DIK_SPACE] & 0x80) != 0;
}

void UserInputReader::getMouseLocationChange(int& mouseX, int& mouseY) const {
    mouseX = this->mouseState.lX;
    mouseY = this->mouseState.lY;
}

void UserInputReader::readKeyboard() {
    HRESULT result = keyboard->GetDeviceState(sizeof(keyboardState), static_cast<LPVOID>(&keyboardState));
    if (FAILED(result)) {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
            keyboard->Acquire();
        }
    }
}

void UserInputReader::readMouse() {
    HRESULT result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), static_cast<LPVOID>(&mouseState));
    if (FAILED(result)) {
        // If the mouse lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
            mouse->Acquire();
        }
    }
}
