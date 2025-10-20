#include "../AtmosphericScattering.hlsli"

struct DirectionalLight {
    float32_t3 color;
    float32_t intensity;
    float32_t3 direction;
};

struct PointLight{
    float32_t3 color;
    float32_t intensity;
    float32_t3 position;
};

struct SpotLight {
    float32_t3 color;
    float32_t intensity;
    float32_t3 position;
    float32_t angleScale;
    float32_t3 direction;
    float32_t angleOffset;
};

struct Scene {
    float32_t4x4 viewProjectionInverseMatrix;
    float32_t3 cameraPosition;
    uint32_t radianceMipCount;
    DirectionalLight directionalLight;
};

ConstantBuffer<Scene> g_Scene : register(b0);
ConstantBuffer<SkyParameter> g_SkyParameter : register(b1);

// G-Buffers
Texture2D<float32_t4> g_Albedo : register(t0);
Texture2D<float32_t2> g_MetallicRoughness : register(t1);
Texture2D<float32_t3> g_Normal : register(t2);
Texture2D<float32_t> g_Depth : register(t3);
TextureCube<float32_t3> g_Irradiance : register(t4);
TextureCube<float32_t3> g_Radiance : register(t5);

SamplerState g_DefaultSampler : register(s0);
SamplerState g_CubeMapSampler : register(s1);
