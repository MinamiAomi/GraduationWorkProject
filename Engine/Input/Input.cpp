#include "Input.h"

#include "Graphics/Core/Helper.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment (lib, "xinput.lib")

Input* Input::GetInstance() {
    static Input instance;
    return &instance;
}

void Input::Initialize(HWND hWnd) {
    hWnd_ = hWnd;

    ASSERT_IF_FAILED(DirectInput8Create(
        GetModuleHandle(nullptr), DIRECTINPUT_HEADER_VERSION,
        IID_IDirectInput8, (void**)directInput_.GetAddressOf(), nullptr));

    ASSERT_IF_FAILED(directInput_->CreateDevice(GUID_SysKeyboard, &keybord_, nullptr));
    ASSERT_IF_FAILED(keybord_->SetDataFormat(&c_dfDIKeyboard));
    ASSERT_IF_FAILED(keybord_->SetCooperativeLevel(hWnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY));

    ASSERT_IF_FAILED(directInput_->CreateDevice(GUID_SysMouse, &mouse_, nullptr));
    ASSERT_IF_FAILED(mouse_->SetDataFormat(&c_dfDIMouse));
    ASSERT_IF_FAILED(mouse_->SetCooperativeLevel(hWnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY));
}

void Input::Update() {
    memcpy(preKeys_, keys_, sizeof(keys_));
    keybord_->Acquire();
    keybord_->GetDeviceState(sizeof(keys_), keys_);

    preMouseState_ = mouseState_;
    mouse_->Acquire();
    mouse_->GetDeviceState(sizeof(mouseState_.state), &mouseState_.state);

    POINT p{};
    GetCursorPos(&p);
    ScreenToClient(hWnd_, &p);
    mouseState_.screenPos = p;

    preXInputState_ = xInputState_;
    XInputGetState(0, &xInputState_);
}
