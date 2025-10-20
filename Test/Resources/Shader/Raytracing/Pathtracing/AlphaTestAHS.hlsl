#include "Pathtracing.hlsli"

// パックされた頂点
// NormalとTangentがR10G10B10A2
struct PackedVertex {
    float32_t3 position;
    uint32_t normal;
    uint32_t tangent;
    float32_t2 texcoord;
};

struct Vertex {
    float32_t3 position;
    float32_t3 normal;
    float32_t2 texcoord;
};

struct Material {
    float32_t3 albedo;
    float32_t metallic;
    float32_t3 emissive;
    float32_t roughness;
    uint32_t albedoMapIndex;
    uint32_t metallicRoughnessMapIndex;
    uint32_t normalMapIndex;
    uint32_t pad;
};

struct MeshProperty {
    Material material;
    uint32_t vertexBufferIndex;
    uint32_t vertexOffset;
    uint32_t indexBufferIndex;
    uint32_t indexOffset;
};

StructuredBuffer<MeshProperty> l_MeshProperties : register(t0, space2);

static StructuredBuffer<PackedVertex> s_VertexBuffer;
static uint32_t s_VertexOffset;
static StructuredBuffer<uint32_t> s_IndexBuffer;
static uint32_t s_IndexOffset;
static Material s_Material;
static Texture2D<float32_t4> s_AlbedoMap;

// メッシュプロパティから初期化する
void InitializeMeshProperty(uint32_t meshPropertyIndex) {
    MeshProperty meshProperty = l_MeshProperties[meshPropertyIndex];

    s_VertexBuffer = ResourceDescriptorHeap[meshProperty.vertexBufferIndex];
    s_VertexOffset = meshProperty.vertexOffset;
    s_IndexBuffer = ResourceDescriptorHeap[meshProperty.indexBufferIndex];
    s_IndexOffset = meshProperty.indexOffset;
    s_Material = meshProperty.material;
    s_AlbedoMap = ResourceDescriptorHeap[s_Material.albedoMapIndex];
}

float32_t3 CalcBarycentrics(in float32_t2 barycentrics) {
    return float32_t3(1.0f - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);
}

// テクスチャ座標を計算
float32_t2 GetTexcoord(in Attributes attributes) {
    float32_t2 texcoord = (float32_t2)0;
    float32_t3 barycentrics = CalcBarycentrics(attributes.barycentrics);
    uint32_t primitiveID = PrimitiveIndex() * 3 + s_IndexOffset;

    for (uint32_t i = 0; i < 3; ++i) {
        uint32_t index = s_IndexBuffer[primitiveID + i] + s_VertexOffset;
        texcoord += s_VertexBuffer[index].texcoord * barycentrics[i];
    }

    return texcoord;
}

float32_t ComputeAlpha(in Attributes attributes) {
    float32_t2 texcoord = GetTexcoord(attributes);
    return s_AlbedoMap.SampleLevel(g_LinearSampler, texcoord, 0).w;
}

[shader("anyhit")]
void AlphaTestAHS(inout Payload payload, in Attributes attributes) {

    // メッシュ情報を取得
    uint32_t meshPropertyIndex = InstanceID() + GeometryIndex();;
    InitializeMeshProperty(meshPropertyIndex);

    // アルファを計算
    float32_t alpha = ComputeAlpha(attributes);


    // アルファが0の場合は衝突を無視
    if (alpha == 0.0f) {
        IgnoreHit();
    }
}