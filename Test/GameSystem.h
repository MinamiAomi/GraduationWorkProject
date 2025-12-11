#pragma once

#include "Scene/BaseGameSystem.h"


class GameSystem :
    public BaseGameSystem {
public:
    enum class PlayDevice {
        KeyboardMouse, // キーボード
        LightDevice // ライトデバイス
    };

    static GameSystem* GetInstance();

    void OnUpdate() override;
    void OnFinalize() override;
    bool IsTerminateSystem() const override;

    void Quit();

    void SetPlayDevice(PlayDevice playDevice) { playDevice_ = playDevice; }
    PlayDevice GetPlayDevice() const { return playDevice_; }

private:
    GameSystem() = default;
    ~GameSystem() = default;
    GameSystem(const GameSystem&) = delete;
    GameSystem& operator=(const GameSystem&) = delete;

    PlayDevice playDevice_ = PlayDevice::KeyboardMouse;
    bool isTerminate_ = false;

};