#pragma once

#include <string>
#include <chrono>

namespace Utility {
    /// <summary>
    /// stringからwstringに変換
    /// </summary>
    /// <param name="str"></param>
    /// <returns></returns>
    std::wstring ConvertString(const std::string& str);
    /// <summary>
    /// wstringからstringに変換
    /// </summary>
    /// <param name="str"></param>
    /// <returns></returns>
    std::string ConvertString(const std::wstring& str);


    class Timer {
    public:
        using Clock = std::chrono::steady_clock;

        Timer() { Start(); }

        void Start() {
            startTime_ = std::chrono::steady_clock::now();
        }
        
        template<typename Duration = std::chrono::milliseconds>
        Duration GetElapsed() const {
            return std::chrono::duration_cast<Duration>(Clock::now() - startTime_);
        }

        template<typename Duration = std::chrono::milliseconds>
        bool HasElapsed(long long value) const {
            return GetElapsed<Duration>() >= Duration(value);
        }


    private:
        std::chrono::steady_clock::time_point startTime_;
    };
}