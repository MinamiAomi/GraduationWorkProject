
struct Scene {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float3 cameraPosition;
    float3 sunLightColor;
    float3 sunLightDirection;
    float sunLightIntensity;
    //uint numDirectionalLights;
    //uint numPointLights;
    //uint numSpotLights;
};
ConstantBuffer<Scene> g_Scene : register(b0);

struct Instance {
    float4x4 worldMatrix;
    float4x4 worldInverseTransposeMatrix;
    float3 color;
    float alpha;
    uint useLighting;
};
ConstantBuffer<Instance> g_Instance : register(b1);

struct Material {
    float3 diffuse;
    float shininess;
    float3 specular;
};
ConstantBuffer<Material> g_Material : register(b2);

Texture2D<float4> g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct DirectionalLight {
    float3 direction;
    float intensity;
    float3 color;
};
StructuredBuffer<DirectionalLight> g_DirectionalLights : register(t1);

//struct PointLight {
//    float3 position;
//    float intensity;
//    float3 color;
//    float radius;
//    float decay;
//};
//StructuredBuffer<PointLight> g_PointLights : register(t2);

//struct SpotLight {
//    float3 direction;
//    float intensity;
//    float3 position;
//    float distance;
//    float3 color;
//    float decay;
//    float cosAngle;
//    float cosFalloffStart;
//};
//StructuredBuffer<SpotLight> g_SpotLights : register(t2);