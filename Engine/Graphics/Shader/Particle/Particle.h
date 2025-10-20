#pragma once

#ifndef HLSL_HEADER
#include <cstdint>
#include "Math/MathUtils.h"

using float32_t = float;
using float32_t2 = Vector2;
using float32_t3 = Vector3;
using float32_t4 = Vector4;

#endif // HLSL_HEADER

#define NUM_THREADS 1024
#define MAX_PARTICLES uint32_t(2 << 12)

#ifndef HLSL_HEADER
namespace GPUParticleStructs {
#endif // !HLSL_HEADER

    struct Particle {
        float32_t3 translate;
        float32_t3 scale;
        float32_t lifeTime;
        float32_t3 velocity;
        float32_t currentTime;
        float32_t4 color;
    };

    struct PerFrame {
        float32_t time;
        float32_t deltaTime;
    };

    struct EmitterParameter {
        float32_t3 translate;
        float32_t radius;
        float32_t3 minScale;
        float32_t minLifeTime;
        float32_t3 maxScale;
        float32_t maxLifeTime;
        float32_t initialSpeed;
        uint32_t count;
    };

#ifndef HLSL_HEADER
} // GPUParticleStructs
#endif // !HLSL_HEADER