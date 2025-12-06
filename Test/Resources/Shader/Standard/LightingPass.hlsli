#include "../AtmosphericScattering.hlsli"

static const uint32_t LIGHT_TYPE_DIRECTIONAL = 0;
static const uint32_t LIGHT_TYPE_POINT = 1;
static const uint32_t LIGHT_TYPE_SPOT = 2;

struct Light {
    float32_t3 color;
    float32_t intensity;
    float32_t3 position;
    float32_t range;
    float32_t3 direction;
    float32_t decay;
    float32_t cosOuter;
    float32_t cosInner;
    float32_t pad1;
    float32_t pad2;
    uint32_t type;
};

struct Scene {
    float32_t4x4 viewProjectionInverseMatrix;
    float32_t3 cameraPosition;
    uint32_t lightCount;
};

ConstantBuffer<Scene> g_Scene : register(b0);
ConstantBuffer<SkyParameter> g_SkyParameter : register(b1);

// Lights
StructuredBuffer<Light> g_Lights : register(t0);

// G-Buffers
Texture2D<float32_t4> g_Albedo : register(t1);
Texture2D<float32_t2> g_MetallicRoughness : register(t2);
Texture2D<float32_t3> g_Normal : register(t3);
Texture2D<float32_t> g_Depth : register(t4);

SamplerState g_DefaultSampler : register(s0);
