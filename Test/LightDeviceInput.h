#pragma once
#include <Windows.h>
#include <atomic>
#include <thread>
#include <mutex>

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
    void Finalize();

    Quaternion GetOrientation() const;
    Vector3 GetAcceleration() const;
    Vector3 GetGyro() const;
    ConnectionState GetConnectionState() const;

private:
    LightDeviceInput() = default;
    ~LightDeviceInput() = default;
    LightDeviceInput(const LightDeviceInput&) = delete;
    LightDeviceInput& operator=(const LightDeviceInput&) = delete;

    void InternalLoad();
    void StartReceiving();
    void CommunicationLoop();

    HANDLE hSerial_;
    
    Vector3 acceleration_;
    Vector3 gyro_;
    Quaternion orientation_;

    mutable std::mutex dataMutex_;
    std::thread communicationThread_;
    std::atomic<bool> isRunning_{ false };
    std::atomic<ConnectionState> connectionState_{ ConnectionState::Disconnected };
};