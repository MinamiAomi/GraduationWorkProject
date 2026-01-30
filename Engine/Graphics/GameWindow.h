#pragma once

#include <Windows.h>
#include <cstdint>

enum class WindowMode {
    Window, // ウィンドウ
    Borderless, // ボーダーレスウィンドウ
    Fullscreen // フルスクリーン
};

class GameWindow {
public:

    static GameWindow* GetInstance();

    void Initialize(const wchar_t* title, uint32_t clientWidth, uint32_t clientHeight);
    bool ProcessMessage() const;
    void Shutdown();

    void SetWindowMode(WindowMode windowMode);
    void SetWindowSize(uint32_t clientWidth, uint32_t clientHeight);

    HWND GetHWND() const { return hWnd_; }
    uint32_t GetClientWidth() const { return clientWidth_; }
    uint32_t GetClientHeight() const { return clientHeight_; }
    WindowMode GetWindowMode() const { return windowMode_; }

private:
    GameWindow() = default;
    GameWindow(const GameWindow&) = delete;
    GameWindow& operator=(const GameWindow&) = delete;

    HWND hWnd_ = nullptr;
    uint32_t clientWidth_ = 0;
    uint32_t clientHeight_ = 0;
    UINT windowStyle_ = 0;
    RECT windowRect_ = {};
    WindowMode windowMode_ = WindowMode::Window;

};