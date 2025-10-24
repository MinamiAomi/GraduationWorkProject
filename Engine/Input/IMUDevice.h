#pragma once
#include <Windows.h>
#include <chrono>

#include "Math/MathUtils.h"

class IMUDevice {
public:
    void Initialize();
    void Update();
    void Finalize();

    const Quaternion& GetOrientation() const { return orientation_; }

private:
    HANDLE hSerial_;
    bool connected_ = false;

    Quaternion orientation_;
};