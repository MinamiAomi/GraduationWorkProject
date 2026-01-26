#include "LightDeviceInput.h"

#include <string>
#include <sstream>
#include <thread>
#include <algorithm>
#include <chrono>

#include "Framework/Engine.h"
#include "Framework/ThreadPool.h"

std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

LightDeviceInput* LightDeviceInput::GetInstance() {
    static LightDeviceInput instance;
    return &instance;
}

void LightDeviceInput::Initialize() {
    // すでに接続されている場合は何もしない
    if (GetConnectionState() != ConnectionState::Disconnected) {
        return;
    }
    connectionState_.store(ConnectionState::Connecting);
    // 非同期でシリアルポートの初期化を行う
    Engine::GetThreadPool()->PushTask([this]() { InternalLoad(); });
}

void LightDeviceInput::StartReceiving() {
    isRunning_.store(true);
    communicationThread_ = std::thread(&LightDeviceInput::CommunicationLoop, this);
}

void LightDeviceInput::Finalize() {
    connectionState_.store(ConnectionState::Disconnected);
    isRunning_.store(false);
    if (communicationThread_.joinable()) {
        communicationThread_.join();
    }
    if (hSerial_ != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial_);
        hSerial_ = INVALID_HANDLE_VALUE;
    }
}

void LightDeviceInput::ResetOrientation() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    resetOrientation_ = orientation_.Inverse();
}

void LightDeviceInput::InternalLoad() {
    connectionState_.store(ConnectionState::Connecting);


    hSerial_ = INVALID_HANDLE_VALUE;
    std::wstring port;

    const std::wstring prefix = L"\\\\.\\COM";
    const uint32_t maxPorts = 16;
    const std::string handshakeSend = "LightDevice\n";
    const std::string handshakeReceive = "LightDevice";
    for (uint32_t i = 0; i < maxPorts; ++i) {
        std::wstring checkingPort = prefix + std::to_wstring(i);

#ifdef _DEBUG
        {
            std::wstring debugText = L"Checking " + checkingPort + L". . .\n";
            OutputDebugStringW(debugText.c_str());
        }
#endif // _DEBUG

        // ポートの接続をしてみる
        HANDLE hPort = CreateFile(checkingPort.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hPort == INVALID_HANDLE_VALUE) {
            continue;
        }

        DCB dcbSerialParams = { 0 };
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hPort, &dcbSerialParams)) {
            CloseHandle(hPort);
            continue;
        }

        dcbSerialParams.BaudRate = 115200;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
        if (!SetCommState(hPort, &dcbSerialParams)) {
            CloseHandle(hPort);
            continue;
        }

        COMMTIMEOUTS timeouts = { 0 };
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 1000;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        timeouts.WriteTotalTimeoutConstant = 500;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        if (!SetCommTimeouts(hPort, &timeouts)) {
            CloseHandle(hPort);
            continue;
        }

        // 1秒待機
        std::this_thread::sleep_for(std::chrono::seconds(1));

        DWORD bytesWritten = 0;
        if (!WriteFile(hPort, handshakeSend.c_str(), (DWORD)handshakeSend.length(), &bytesWritten, NULL)) {
            CloseHandle(hPort);
            continue;
        }

        char readBuffer[256] = { 0 };
        DWORD bytesRead = 0;
        std::string response;

        while (ReadFile(hPort, readBuffer, 1, &bytesRead, NULL) && bytesRead > 0) {
            if (readBuffer[0] == '\n') {
                break;
            }
            response += readBuffer[0];
        }

        std::erase_if(response, [](char c) { return c == '\n' || c == '\r'; });

        if (response == handshakeReceive) {
#ifdef _DEBUG
            {
                std::wstring debugText = L"Device found at " + checkingPort + L"!\n";
                OutputDebugStringW(debugText.c_str());
            }
#endif // _DEBUG
            hSerial_ = hPort;
            break;
        }

        CloseHandle(hPort);
    }

    bool success = (hSerial_ != INVALID_HANDLE_VALUE);

    if (success) {
        connectionState_.store(ConnectionState::Connected);
        StartReceiving();
    }
    else {
        connectionState_.store(ConnectionState::Disconnected);
    }
}

void LightDeviceInput::CommunicationLoop() {
    while (isRunning_.load()) {
        std::string response;
        char readBuffer[256] = {};
        DWORD bytesRead;

        while (ReadFile(hSerial_, readBuffer, 1, &bytesRead, NULL) && bytesRead > 0) {
            if (readBuffer[0] == '\n') {
                break;
            }
            response += readBuffer[0];
        }

        std::erase_if(response, [](char c) { return c == '\n' || c == '\r'; });

        auto data = Split(response, ',');

        Quaternion orientation;
        if (data.size() >= 4) {

            orientation.w = std::stof(data[0]);
            orientation.x = std::stof(data[1]);
            orientation.y = std::stof(data[2]);
            orientation.z = std::stof(data[3]);
            if (orientation.LengthSquare() != 0.0f) {
                orientation = orientation.Normalized();
            }
        }

        std::lock_guard<std::mutex> lock(dataMutex_);

        orientation_.z = orientation.w;
        orientation_.x = -orientation.x;
        orientation_.y = -orientation.z;
        orientation_.z = orientation.y;
    }

}

Quaternion LightDeviceInput::GetOrientation() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return orientation_ * resetOrientation_;
}

LightDeviceInput::ConnectionState LightDeviceInput::GetConnectionState() const {
    return connectionState_.load();
}