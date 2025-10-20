#pragma once

#include <chrono>

class Timer {
public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();
    /// <summary>
    /// フレームレートをキープ
    /// </summary>
    /// <param name="fps"></param>
    void KeepFrameRate(uint32_t fps);

private:
    std::chrono::steady_clock::time_point reference_;
};