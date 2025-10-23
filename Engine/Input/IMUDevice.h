#pragma once
#include <Windows.h>
#include <chrono>

#include "Math/MathUtils.h"

class IMUDevice {
public:
    void Initialize();
    void Update();
    void Finalize();

    const Vector3& GetAcceleration() const { return acceleration_; }
    const Vector3& GetGyroscope() const { return gyroscope_; }
    const Vector3& GetMagnetometer() const { return magnetometer_; }

private:
    HANDLE hSerial_;
    bool connected_ = false;

    Vector3 acceleration_;
    Vector3 gyroscope_;
    Vector3 magnetometer_;

};