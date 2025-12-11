#pragma once
#include <Windows.h>
#include <atomic>

#include "Math/MathUtils.h"

class LightDeviceInput {
public:
    enum class ConnectionState {
        // 未接続
        Disconnected,
        // 接続中
        Connecting,
        // 接続完了
        Connected
    };

    static LightDeviceInput* GetInstance();

    void Initialize();
    void Update();
    void Finalize();

    const Quaternion& GetOrientation() const;
    ConnectionState GetConnectionState() const;

private:
    LightDeviceInput() = default;
    ~LightDeviceInput() = default;
    LightDeviceInput(const LightDeviceInput&) = delete;
    LightDeviceInput& operator=(const LightDeviceInput&) = delete;

    void InternalLoad();

    HANDLE hSerial_;
    Quaternion orientation_;
    std::atomic<ConnectionState> connectionState_{ ConnectionState::Disconnected };
};