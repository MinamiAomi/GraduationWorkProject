struct Well {
    float32_t4x4 skeletonSpaceMatrix;
    float32_t4x4 skeletonSpaceInverseTransposeMatrix;
};

struct Vertex {
    float32_t3 position;
    uint32_t normal;
    uint32_t tangent;
    float32_t2 texcoord;
};

struct VertexInfluence {
    int32_t4 index;
    float32_t4 weight;
};

struct SkinningInformation {
    uint32_t numVertices;
};

StructuredBuffer<Well> g_MatrixPalette : register(t0);
StructuredBuffer<Vertex> g_InputVertices : register(t1);
StructuredBuffer<VertexInfluence> g_Influence : register(t2);
RWStructuredBuffer<Vertex> g_OutputVertices : register(u0);
ConstantBuffer<SkinningInformation> g_SkinningInformation : register(b0);

float32_t4 R10G10B10A2ToFloat4(uint32_t value) {
    float32_t x = (float32_t)((value >> 0) & 0x3FF) / 1023.0f;
    float32_t y = (float32_t)((value >> 10) & 0x3FF) / 1023.0f;
    float32_t z = (float32_t)((value >> 20) & 0x3FF) / 1023.0f;
    float32_t w = (float32_t)((value >> 30) & 0x3) / 1023.0f;
    return float32_t4(x, y, z, w);
}

uint32_t Float4ToR10G10B10A2(float32_t4 value) {
    uint32_t x = (uint32_t)(value.x * 1023.0f);
    uint32_t y = (uint32_t)(value.y * 1023.0f);
    uint32_t z = (uint32_t)(value.z * 1023.0f);
    uint32_t w = (uint32_t)(value.w * 3.0f);
    return (x << 0) | (y << 10) | (z << 20) | (w << 30);
}

float32_t4x4 Identity() {
    return float32_t4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

[numthreads(1024, 1, 1)]
void main(uint32_t3 DTid : SV_DispatchThreadID) {

    uint32_t vertexIndex = DTid.x;
    // 頂点インデックスがバッファをオーバーしないように
    if (vertexIndex >= g_SkinningInformation.numVertices) { return; }

    Vertex input = g_InputVertices[vertexIndex];
    VertexInfluence influence = g_Influence[vertexIndex];

    Vertex skinned;
    skinned.texcoord = input.texcoord;

    float32_t4 position;
    position  = mul(float32_t4(input.position, 1.0f), g_MatrixPalette[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
    position += mul(float32_t4(input.position, 1.0f), g_MatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
    position += mul(float32_t4(input.position, 1.0f), g_MatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
    position += mul(float32_t4(input.position, 1.0f), g_MatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
    skinned.position = position.xyz;

    float32_t3 normal, originalNormal = R10G10B10A2ToFloat4(input.normal).xyz * 2.0f - 1.0f;
    normal  = mul(originalNormal, (float32_t3x3)g_MatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
    normal += mul(originalNormal, (float32_t3x3)g_MatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
    normal += mul(originalNormal, (float32_t3x3)g_MatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
    normal += mul(originalNormal, (float32_t3x3)g_MatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
    normal = (normalize(normal) + 1.0f) * 0.5f;
    skinned.normal = Float4ToR10G10B10A2(float32_t4(normal, 0.0f));

    float32_t3 tangent, originalTangent = R10G10B10A2ToFloat4(input.tangent).xyz * 2.0f - 1.0f;
    tangent  = mul(originalTangent, (float32_t3x3)g_MatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
    tangent += mul(originalTangent, (float32_t3x3)g_MatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
    tangent += mul(originalTangent, (float32_t3x3)g_MatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
    tangent += mul(originalTangent, (float32_t3x3)g_MatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
    tangent = (normalize(tangent) + 1.0f) * 0.5f;
    skinned.tangent = Float4ToR10G10B10A2(float32_t4(tangent, 0.0f));

    g_OutputVertices[vertexIndex] = skinned;
}