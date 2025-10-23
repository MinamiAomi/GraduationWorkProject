#include "IMUDevice.h"

#include <string>
#include <sstream>

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

    auto port = L"\\\\.\\COM5";
    // 接続
    hSerial_ = CreateFile(port, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    assert(hSerial_ != INVALID_HANDLE_VALUE);

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial_, &dcbSerialParams)) {
        assert(true);
    }

    dcbSerialParams.BaudRate = 115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial_, &dcbSerialParams)) {
        assert(true);
    }

    connected_ = true;
}


void IMUDevice::Update() {
    std::string line;
    char buffer[2] = {};
    DWORD bytesRead;

    while (true) {
        if (ReadFile(hSerial_, buffer, 1, &bytesRead, NULL)) {
            if (bytesRead > 0) {
                if (buffer[0] == '\n') {
                    break;
                }
                line += buffer[0];
            }
        }
        else {
            assert(true);
        }
    }

    auto data = Split(line, ',');

    if (data.size() >= 9) {
        acceleration_ = { std::stof(data[0]), std::stof(data[1]), std::stof(data[2]) };
        gyroscope_ = { std::stof(data[3]), std::stof(data[4]), std::stof(data[5]) };
        magnetometer_ = { std::stof(data[6]), std::stof(data[7]), std::stof(data[8]) };
    }
}

void IMUDevice::Finalize() {
    if (hSerial_ != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial_);
        hSerial_ = INVALID_HANDLE_VALUE;
    }
}