#include "Sky.h"
#ifdef ENABLE_IMGUI
#include "ImGuiManager.h"
#endif ENABLE_IMGUI

Vector3 CalcSunDirection(uint32_t day, float time, float northLatitude, float eastLongitude) {
    static const float omega = Math::TwoPi / 365.0f;
    static const float J = day + 0.5f;
    static const float omegaJ = omega * J;
    static const float sigma = (0.33281f - 22.984f * std::cos(omegaJ) - 0.34990f * std::cos(2.0f * omegaJ) - 0.13980f * std::cos(3.0f * omegaJ) + 3.7872f * std::sin(omegaJ) + 0.03250f * std::sin(2.0f * omegaJ) + 0.07187f * std::sin(3.0f * omegaJ)) * Math::ToRadian;
    static const float e = 0.0072f * std::cos(omega * J) - 0.0528f * std::cos(2.0f * omegaJ) - 0.0012f * std::cos(3.0f * omegaJ) - 0.1229f * std::sin(omegaJ) - 0.1565f * std::sin(2.0f * omegaJ) - 0.0041f * std::sin(3.0f * omegaJ);

    const float T = time + (eastLongitude - 135.0f) / 15.0f + e;
    const float t = (15.0f * T - 180.0f) * Math::ToRadian;

    const float phi = northLatitude * Math::ToRadian;
    const float h = std::asin(std::sin(phi) * std::sin(sigma) + std::cos(phi) * std::cos(sigma) * std::cos(t));

    const float sinA = std::cos(sigma) * std::sin(t) / std::cos(h);
    const float cosA = (std::sin(h) * std::sin(phi) - std::sin(sigma)) / std::cos(h) / std::cos(phi);

    const float y = std::sin(h);
    const float xzLength = std::sqrt(1.0f - y * y);
    const float z = cosA * xzLength;
    const float x = sinA * xzLength;
    return { x, y, z };
}

void Sky::Update(float deltaTime) {
   
    if (!stoped_) {
        time_ += deltaTime;
    }

    if (time_ >= 24.0f) {
        time_ -= 24.0f;
        ++day_;
        if (day_ >= 366.0f) {
            day_ = 0;
        }
    }

    sunDirection_ = CalcSunDirection(day_, time_, northLatitude_, eastLongitude_).Normalized();
}

void Sky::DrawImGui() {
#ifdef ENABLE_IMGUI
    ImGui::Checkbox("Stop", &stoped_);
    int day = (int)day_;
    ImGui::SliderInt("Day", &day, 0, 356);
    day_ = (uint32_t)day;
    ImGui::SliderFloat("Time", &time_, 0.0f, 24.0f);
    float rad = northLatitude_ * Math::ToRadian;
    ImGui::SliderAngle("North Latitude", &rad, 0.0f, 360.0f);
    northLatitude_ = rad * Math::ToDegree;
    rad = eastLongitude_ * Math::ToRadian;
    ImGui::SliderAngle("East Longitude", &rad, 0.0f, 360.0f);
    eastLongitude_ = rad * Math::ToDegree;
    ImGui::InputFloat3("Sun Direction", &sunDirection_.x);
    Update(0.0f);
#endif ENABLE_IMGUI
}
