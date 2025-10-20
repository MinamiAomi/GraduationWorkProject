#include "Particle.hlsli"

bool IsDead(Particle particle) {
    return particle.currentTime >= particle.lifeTime;
}

[numthreads(NUM_THREADS, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {
    uint32_t particleIndex = DTid.x;
    // 生成できない
    if (particleIndex >= MAX_PARTICLES) { return; }
    if (g_Particles[particleIndex].lifeTime <= 0.0f) { return; }

    g_Particles[particleIndex].translate += g_Particles[particleIndex].velocity;
    g_Particles[particleIndex].currentTime += g_PerFrame.deltaTime;
    g_Particles[particleIndex].color.a = 1.0f - (g_Particles[particleIndex].currentTime / g_Particles[particleIndex].lifeTime);
    
    if (IsDead(g_Particles[particleIndex])) {
        g_Particles[particleIndex].scale = float32_t3(0.0f, 0.0f, 0.0f);
        g_Particles[particleIndex].color = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
        g_Particles[particleIndex].lifeTime = 0.0f;
        PushParticleIndex(particleIndex);
    }
}