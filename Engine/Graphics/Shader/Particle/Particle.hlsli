#define HLSL_HEADER
#include "Particle.h"

ConstantBuffer<PerFrame> g_PerFrame : register(b0, space1);

RWStructuredBuffer<Particle> g_Particles : register(u0, space1);
RWStructuredBuffer<int32_t> g_FreeListIndex : register(u1, space1);
RWStructuredBuffer<uint32_t> g_FreeList : register(u2, space1);

void InitializeFreeList(uint32_t particleIndex) {
    if (particleIndex == 0) {
        g_FreeListIndex[0] = MAX_PARTICLES - 1;
    }
    g_FreeList[particleIndex] = particleIndex;
}

uint32_t PopParticleIndex() {
    int32_t freeListIndex;
    InterlockedAdd(g_FreeListIndex[0], -1, freeListIndex);
    if (0 <= freeListIndex && freeListIndex < MAX_PARTICLES) {
        uint32_t particleIndex = g_FreeList[freeListIndex];
        return particleIndex;
    }
    InterlockedAdd(g_FreeListIndex[0], 1);
    return MAX_PARTICLES;
}

void PushParticleIndex(uint32_t particleIndex) {
    int32_t freeListIndex;
    InterlockedAdd(g_FreeListIndex[0], 1, freeListIndex);
    if ((freeListIndex + 1) < MAX_PARTICLES) {
        g_FreeList[freeListIndex + 1] = particleIndex;
        return;
    }
    InterlockedAdd(g_FreeListIndex[0], -1, freeListIndex);
}