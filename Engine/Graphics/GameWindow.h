#pragma once

#include <Windows.h>
#include <cstdint>

class GameWindow {
public:
    enum class SizeChangeMode {
        kNone,       // サイズ変更不可
        kNormal,     // 自由変更
        kFixedAspect // アスペクトひ一定
    };
    
    static GameWindow* GetInstance();

    void Initialize(const wchar_t* title, uint32_t clientWidth, uint32_t clientHeight);
    bool ProcessMessage() const;
    void Shutdown();

    void SetFullScreen(bool fullScreen);
    bool IsFullScreen() const { return isFullScreen_; };
    void SetSizeChangeMode(SizeChangeMode sizeChangeMode);
    SizeChangeMode GetSizeChangeMode() const { return sizeChangeMode_; }

    HWND GetHWND() const { return hWnd_; }
    uint32_t GetClientWidth() const { return clientWidth_; }
    uint32_t GetClientHeight() const { return clientHeight_; }

private:
    GameWindow() = default;
    GameWindow(const GameWindow&) = delete;
    GameWindow& operator=(const GameWindow&) = delete;

    HWND hWnd_ = nullptr;
    uint32_t clientWidth_ = 0;
    uint32_t clientHeight_ = 0;
    UINT windowStyle_ = 0;
    RECT windowRect_ = {};
    float aspectRaito_ = 0.0f;
    SizeChangeMode sizeChangeMode_ = SizeChangeMode::kFixedAspect;
    bool isFullScreen_ = false;

};