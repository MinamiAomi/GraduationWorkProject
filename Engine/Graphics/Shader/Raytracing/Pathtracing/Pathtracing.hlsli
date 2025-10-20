#include "Pathtracing.h"

#define TRUE_UINT 1
#define FALSE_UINT 0

#define INVALID_COLOR float32_t3(-1.0f, -1.0f, -1.0f)

//#define USE_NORMAL_MAPS

//////////////////////////////////////////////////
// R + M * G + I HitGroupShaderRecordIndex      
// R = RayContributionToHitGroupIndex
// M = MultiplierForGeometryContributionToHitGroupIndex
// G = GeometryContributionToHitGroupIndex
// I = InstanceContributionToHitGroupIndex
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// 0 + 1 * G + I PrimaryHitGroup
#define RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX 0
#define MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX 0
#define MISS_SHADER_INDEX 0
//////////////////////////////////////////////////


#define RAY_ATTRIBUTE (1 << 0)

struct Scene {
    float32_t4x4 viewProjectionInverseMatrix;
    float32_t3 cameraPosition;
    int32_t frame;
};

struct Payload {
    float32_t3 color;
    uint32_t recursiveCount;
    int32_t seed;
};

struct Attributes {
    float32_t2 barycentrics;
};

ConstantBuffer<Scene> g_Scene : register(b0);

RaytracingAccelerationStructure g_TLAS : register(t0);
RWTexture2D<float32_t4> g_Color : register(u0);
SamplerState g_PointSampler : register(s0);
SamplerState g_LinearSampler : register(s1);

Texture2D<float32_t4> g_BindlessTextures[] : register(t0, space1);

int32_t Srand(in int32_t2 xy, in int32_t frame) {
    int32_t n = frame;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    n += xy.y;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    n += xy.x;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    return n;
}

int32_t Rand(inout int32_t seed) {
    seed = seed * 0x343fd + 0x269ec3;
    return (seed >> 16) & 32767;
}

float32_t fRand(inout int32_t seed) {
    return float(Rand(seed)) / 32767.0f;
}