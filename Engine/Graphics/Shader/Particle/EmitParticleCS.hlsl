#include "Particle.hlsli"
#include "../Convert.hlsli"
#include "../Random.hlsli"

ConstantBuffer<EmitterParameter> g_EmitterParameter : register(b0);

[numthreads(NUM_THREADS, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    // 生成しているパーティクルのインデックス
    uint32_t emitParticleIndex = DTid.x;
    if (emitParticleIndex >= g_EmitterParameter.count) { return; }
    // フリーリストから使えるインデックスをとってくる
    uint32_t particleIndex = PopParticleIndex();
    // 生成できない
    if (particleIndex >= MAX_PARTICLES) { return; }

    RandomGenerator rg;
    rg.seed = (DTid + g_PerFrame.time) * g_PerFrame.time;

    g_Particles[particleIndex] = (Particle)0;
    g_Particles[particleIndex].scale = lerp(g_EmitterParameter.minScale, g_EmitterParameter.maxScale, rg.Generate1d());
    g_Particles[particleIndex].velocity = normalize(rg.Generate3d() * 2.0f - 1.0f) * g_EmitterParameter.initialSpeed;
    g_Particles[particleIndex].translate = g_EmitterParameter.translate + normalize(rg.Generate3d()) * g_EmitterParameter.radius * rg.Generate1d();
    g_Particles[particleIndex].color.rgb = HSVToRGB(float32_t3(rg.Generate1d(), 1.0f, 1.0f));
    g_Particles[particleIndex].color.a = 1.0f;
    g_Particles[particleIndex].lifeTime = lerp(g_EmitterParameter.minLifeTime, g_EmitterParameter.maxLifeTime, rg.Generate1d());
}