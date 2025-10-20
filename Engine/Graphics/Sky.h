#pragma once

#include <Math/MathUtils.h>

class Sky {
public:
    /// <summary>
    /// 更新
    /// </summary>
    /// <param name="deltaTime"></param>
    void Update(float deltaTime);

    /// <summary>
    /// ImGuiに描画
    /// </summary>
    void DrawImGui();

    // セッター

    void SetDay(uint32_t day) { day_ = day; }
    void SetTime(float time) { time_ = time; }
    void SetNorthLatitude(float nothLatitude) { northLatitude_ = nothLatitude; }
    void SetEastLongitude(float eastLongitude) { eastLongitude_ = eastLongitude; }
    void SetStoped(float stoped) { stoped_ = stoped; }

    // ゲッター

    const Vector3& GetSunDirection() const { return sunDirection_; }

private:
    uint32_t day_ = 0; // 日
    float time_ = 12.0f; // 時間 h
    float northLatitude_ = 35.0f; // 北緯°
    float eastLongitude_ = 135.0f; // 東経°
    Vector3 sunDirection_;
    bool stoped_ = true;
};