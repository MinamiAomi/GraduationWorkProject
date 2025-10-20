Texture2D<float4> g_BindlessTextures[] : register(t0, space1);
SamplerState g_Sampler : register(s0);

struct Scene {
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float3 cameraPosition;
    float nearClip;
    float farClip;
};
ConstantBuffer<Scene> g_Scene : register(b0);

struct Instance {
    float4x4 worldMatrix;
    float4x4 worldInverseTransposeMatrix;
};
ConstantBuffer<Instance> g_Instance : register(b1);

struct Material {
    float3 albedo;
    float metallic;
    float roughness;
    uint albedoMapIndex;
    uint metallicRoughnessMapIndex;
    uint normalMapIndex;
};
ConstantBuffer<Material> g_Material : register(b2);

#ifdef ENABLE_SKINNING
struct Bone {
    float4x4 boneMatrix;
    float4x4 boneInverseTransposeMatrix;
};
StructuredBuffer<Bone> g_Bones : register(t0);
#endif // ENABLE_SKINNING