#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <Xinput.h>
#include <wrl/client.h>

#include <cstdint>

#define MOUSE_BUTTON_LEFT  0
#define MOUSE_BUTTON_RIGHT 1
#define MOUSE_BUTTON_MID   2

#define MOUSE_CLICKED_VALUE 0x80

class Input {
public:
    /// <summary>
    /// シングルトンインスタンスを取得
    /// </summary>
    /// <returns></returns>
    static Input* GetInstance();
    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="hWnd"></param>
    void Initialize(HWND hWnd);
    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 押された
    /// </summary>
    /// <param name="keycode"></param>
    /// <returns></returns>
    bool IsKeyPressed(unsigned char keycode) const { return keys_[keycode] != 0; }
    /// <summary>
    /// 一瞬押された
    /// </summary>
    /// <param name="keycode"></param>
    /// <returns></returns>
    bool IsKeyTrigger(unsigned char keycode) const { return keys_[keycode] != 0 && preKeys_[keycode] == 0; }
    /// <summary>
    /// 離された
    /// </summary>
    /// <param name="keycode"></param>
    /// <returns></returns>
    bool IsKeyRelease(unsigned char keycode) const { return keys_[keycode] == 0 && preKeys_[keycode] != 0; }

    /// <summary>
    /// 押された
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    bool IsMousePressed(int button) const { return mouseState_.state.rgbButtons[button] & MOUSE_CLICKED_VALUE; }
    /// <summary>
    /// 一瞬押された
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    bool IsMouseTrigger(int button) const { return (mouseState_.state.rgbButtons[button] & MOUSE_CLICKED_VALUE) && !(preMouseState_.state.rgbButtons[button] & MOUSE_CLICKED_VALUE); }
    /// <summary>
    /// 離された
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    bool IsMouseRelease(int button) const { return !(mouseState_.state.rgbButtons[button] & MOUSE_CLICKED_VALUE) && (preMouseState_.state.rgbButtons[button] & MOUSE_CLICKED_VALUE); }
    /// <summary>
    /// マウス座標を取得
    /// </summary>
    /// <returns></returns>
    POINT GetMousePosition() const { return mouseState_.screenPos; }

    /// <summary>
    /// マウスのX軸の変化量
    /// </summary>
    /// <returns></returns>
    LONG GetMouseMoveX() const { return mouseState_.state.lX; }
    /// <summary>
    /// マウスのY軸の変化量
    /// </summary>
    /// <returns></returns>
    LONG GetMouseMoveY() const { return mouseState_.state.lY; }
    /// <summary>
    /// マウスホイール
    /// </summary>
    /// <returns></returns>
    LONG GetMouseWheel() const { return mouseState_.state.lZ; }

    /// <summary>
    /// ゲームパッドを取得
    /// </summary>
    /// <returns></returns>
    const XINPUT_STATE& GetXInputState() const { return xInputState_; }
    /// <summary>
    /// 前フレームのゲームパッドを取得
    /// </summary>
    /// <returns></returns>
    const XINPUT_STATE& GetPreXInputState() const { return preXInputState_; }

private:
    static const uint32_t kNumKeys = 256;
   
    Input() = default;
    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    struct MouseState {
        DIMOUSESTATE state;
        POINT screenPos;
    };

    HWND hWnd_ = nullptr;
    Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> keybord_;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_;

    uint8_t keys_[kNumKeys]{};
    uint8_t preKeys_[kNumKeys]{};

    MouseState mouseState_{};
    MouseState preMouseState_{};

    XINPUT_STATE xInputState_{};
    XINPUT_STATE preXInputState_{};
};

#ifdef MOUSE_CLICKED_VALUE
#undef MOUSE_CLICKED_VALUE
#endif // MOUSE_CLICKED_VALUE
