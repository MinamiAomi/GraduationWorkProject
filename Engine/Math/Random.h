#pragma once

#include <cstdint>
#include <random>

namespace Random {

    class RandomNumberGenerator {
    public:
        RandomNumberGenerator(uint32_t seed = 0) : engine_((seed == 0) ? seedGenerator_() : seed) {}

        /// <summary>
        /// シードをセット
        /// </summary>
        /// <param name="seed"></param>
        void SetSeed(uint32_t seed) { engine_.seed(seed); }

        /// <summary>
        /// 範囲指定して取得
        /// </summary>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        int32_t NextIntRange(int32_t min, int32_t max) { return std::uniform_int_distribution<int32_t>(min, max)(engine_); }
        /// <summary>
        /// intの最小値から最大値を取得
        /// </summary>
        /// <returns></returns>
        int32_t NextIntLimit() { return NextIntRange(INT_MIN, INT_MAX); }

        /// <summary>
        /// 範囲指定して取得
        /// </summary>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        uint32_t NextUIntRange(uint32_t min, uint32_t max) { return std::uniform_int_distribution<uint32_t>(min, max)(engine_); }
        /// <summary>
        /// uintの最小値から最大値を取得
        /// </summary>
        /// <returns></returns>
        uint32_t NextUIntLimit() { return NextUIntRange(0u, UINT_MAX); }

        /// <summary>
        /// 範囲指定して取得
        /// </summary>
        /// <param name="min"></param>
        /// <param name="max"></param>
        /// <returns></returns>
        float NextFloatRange(float min, float max) { return std::uniform_real_distribution<float>(min, max)(engine_); }
        /// <summary>
        /// floatの最小値から最大値を取得
        /// </summary>
        /// <returns></returns>
        float NextFloatLimit() { return NextFloatRange(FLT_MIN, FLT_MAX); }
        /// <summary>
        /// 0~1を取得
        /// </summary>
        /// <returns></returns>
        float NextFloatUnit() { return NextFloatRange(0.0f, 1.0f); }

    private:
        std::random_device seedGenerator_;
        std::mt19937 engine_;
    };

}