#include "Particle.hlsli"

[numthreads(NUM_THREADS, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    uint32_t particleIndex = DTid.x;

    // 早期リターン
    if (particleIndex >= MAX_PARTICLES) { return; }
    
    // フリーリストを初期化
    InitializeFreeList(particleIndex);

    g_Particles[particleIndex] = (Particle)0;
}