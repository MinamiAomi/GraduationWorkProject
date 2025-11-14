#include "IMUDevice.h"

#include <string>
#include <sstream>
#include <thread>
#include <algorithm>
#include <chrono>

std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

void IMUDevice::Initialize() {

    hSerial_ = INVALID_HANDLE_VALUE;
    std::wstring port;

    const std::wstring prefix = L"\\\\.\\COM";
    const uint32_t maxPorts = 16;
    const std::string handshakeSend = "FlashlightConnecting\n";
    const std::string handshakeReceive = "FlashlightConnected";
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

        std::this_thread::sleep_for(std::chrono::seconds(2));

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
    assert(hSerial_ != INVALID_HANDLE_VALUE);
    connected_ = true;
}


void IMUDevice::Update() {
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

    if (data.size() >= 4) {
        orientation_.w = -std::stof(data[0]);
        orientation_.x = std::stof(data[1]);
        orientation_.y = std::stof(data[3]);
        orientation_.z = std::stof(data[2]);
    }
}

void IMUDevice::Finalize() {
    if (hSerial_ != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial_);
        hSerial_ = INVALID_HANDLE_VALUE;
    }
}