#include <Arduino.h>
#include <Wire.h>
#include <pinout.h>
#include <gpio.h>
#include <AK09918.h>
#include <ICM20600.h>
#include <MadgwickAHRS.h>
#include <vector3.h>

class IMU9DOF {
public:
    ICM20600 icm20600;
    AK09918 ak09918;

    Vector3 gyroBias;
    bool calibrated = false;

    Vector3 magMin = { -1000.0f, -1000.0f, -1000.0f};
    Vector3 magMax = { 1000.0f, 1000.0f, 1000.0f};
    Vector3 magOffset = { 0.0f, 0.0f, 0.0f };
    int sampleCount = 0;

    void initialize() {
        Wire.begin();
        icm20600.initialize();
        ak09918.initialize();
        ak09918.switchMode(AK09918_CONTINUOUS_100HZ);

        //delay(100);
        //calibrateGyro();
    }

    void calibrate() {
        const int maxSampleCount = 1000; 
        if (sampleCount >= maxSampleCount) {
            return;
        }
        sampleCount++;
        int32_t x, y, z;
        if (ak09918.getData(&x, &y, &z) == AK09918_ERR_OK) {
            if (x > magMax.x) magMax.x = x; if (x < magMin.x) magMin.x = x;
            if (y > magMax.y) magMax.y = y; if (y < magMin.y) magMin.y = y;
            if (z > magMax.z) magMax.z = z; if (z < magMin.z) magMin.z = z;
        }
        magOffset.x = (magMax.x + magMin.x) * 0.5f;
        magOffset.y = (magMax.y + magMin.y) * 0.5f;
        magOffset.z = (magMax.z + magMin.z) * 0.5f;
    }

    Vector3 getAcceleration() {
        Vector3 a;
        a.x = (float)icm20600.getAccelerationX();
        a.y = (float)icm20600.getAccelerationY();
        a.z = (float)icm20600.getAccelerationZ();
        return a;
    }

    Vector3 getGyroscope() {
        Vector3 g;
        //float rawX = ((float)icm20600.getRawGyroscopeX() - gyroBias.x) / 16.4f;
        //float rawY = ((float)icm20600.getRawGyroscopeY() - gyroBias.y) / 16.4f;
        //float rawZ = ((float)icm20600.getRawGyroscopeZ() - gyroBias.z) / 16.4f;
        g.x = (float)icm20600.getGyroscopeX();
        g.y = (float)icm20600.getGyroscopeY();
        g.z = (float)icm20600.getGyroscopeZ();
        return g;
    }

    Vector3 getMagnetometer() {
        Vector3 m;
        int32_t x, y, z;
        if (ak09918.getData(&x, &y, &z) == AK09918_ERR_OK) {
            m.x = (float)x/*- magOffset.x*/;
            m.y = (float)y/*- magOffset.y*/;
            m.z = (float)z/*- magOffset.z*/;
        }
        return m;
    }
};

Gpio* sw = nullptr;
IMU9DOF imu;
Madgwick filter;
bool connected = false;
unsigned long lastTime = 0;

void setup() {
    Serial.begin(115200);
    sw = new Gpio(Pinout::D2);
    imu.initialize();

    filter.begin(60.0f);
    
    delay(60);
    
    
    lastTime = micros();
}

void loop() {
    if (!connected && Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        if (command == "LightDevice") {
            Serial.println("LightDevice");
            connected = true;
        } 
    }
    
    unsigned long currentTime  = micros();
    float dt = (float)(currentTime - lastTime) / 1000000.0f;
    lastTime = currentTime;

    if (dt <= 0 || dt > 0.1f) dt = 0.01f;

    Vector3 a = imu.getAcceleration();
    Vector3 g = imu.getGyroscope();

    filter.updateIMU(g.x, g.y, g.z, a.x, a.y, a.z, dt);

    const bool isPressed = sw->input();

    if (connected) {

        float qw, qx, qy, qz;
        filter.getQuaternion(&qw, &qx, &qy, &qz);
        
        Serial.print(qw, 4); Serial.print(",");
        Serial.print(qx, 4); Serial.print(",");
        Serial.print(qy, 4); Serial.print(",");
        Serial.print(qz, 4); Serial.print(",");

        Serial.println(isPressed ? "T" : "F"); 
    }
}

