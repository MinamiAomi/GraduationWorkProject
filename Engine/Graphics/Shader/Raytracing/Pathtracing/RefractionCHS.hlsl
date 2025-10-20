#include "Pathtracing.hlsli"
#include "../../PBR.hlsli"
#include "../../Random.hlsli"

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
static Texture2D<float32_t3> s_MetallicRoughnessMap;
static Texture2D<float32_t3> s_NormalMap;
// メッシュプロパティから初期化する
void InitializeMeshProperty(uint32_t meshPropertyIndex) {
    MeshProperty meshProperty = l_MeshProperties[meshPropertyIndex];

    s_VertexBuffer = ResourceDescriptorHeap[meshProperty.vertexBufferIndex];
    s_VertexOffset = meshProperty.vertexOffset;
    s_IndexBuffer = ResourceDescriptorHeap[meshProperty.indexBufferIndex];
    s_IndexOffset = meshProperty.indexOffset;
    s_Material = meshProperty.material;
    s_AlbedoMap = ResourceDescriptorHeap[s_Material.albedoMapIndex];
    s_MetallicRoughnessMap = ResourceDescriptorHeap[s_Material.metallicRoughnessMapIndex];
    s_NormalMap = ResourceDescriptorHeap[s_Material.normalMapIndex];
}

float32_t3 CalcBarycentrics(in float32_t2 barycentrics) {
    return float32_t3(1.0f - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);
}

float32_t4 R10G10B10A2Tofloat32_t4(in uint32_t value) {
    float32_t x = (float32_t)((value >> 0) & 0x3FF) / 1023.0f;
    float32_t y = (float32_t)((value >> 10) & 0x3FF) / 1023.0f;
    float32_t z = (float32_t)((value >> 20) & 0x3FF) / 1023.0f;
    float32_t w = (float32_t)((value >> 30) & 0x3) / 1023.0f;
    return float32_t4(x, y, z, w) * 2.0f - 1.0f;
}
// 法線を計算
float32_t3 GetNormal(in float32_t3 normal, in float32_t3 tangent, in float32_t2 texcoord) {
    // 法線マップからサンプリング
    float32_t3 normalMap = s_NormalMap.SampleLevel(g_PointSampler, texcoord, 0).xyz;
    // UNORMからSNORMに変換
    normalMap = normalMap * 2.0f - 1.0f;
    // NormalとTangentに垂直なベクトル
    float32_t3 binormal = normalize(cross(tangent, normal));
    // 新しい法線
    float32_t3 newNormal = normalMap.x * tangent + normalMap.y * binormal + normalMap.z * normal;
    return newNormal;
}
// ワールド座標の頂点を計算
Vertex GetVertex(in Attributes attributes) {
    Vertex vertex = (Vertex)0;
    float32_t3 barycentrics = CalcBarycentrics(attributes.barycentrics);
    uint32_t primitiveID = PrimitiveIndex() * 3 + s_IndexOffset;

    float32_t3 normal = float32_t3(0.0f, 0.0f, 0.0f), tangent = float32_t3(0.0f, 0.0f, 0.0f);
    for (uint32_t i = 0; i < 3; ++i) {
        uint32_t index = s_IndexBuffer[primitiveID + i] + s_VertexOffset;
        vertex.position += s_VertexBuffer[index].position * barycentrics[i];
        normal += R10G10B10A2Tofloat32_t4(s_VertexBuffer[index].normal).xyz * barycentrics[i];
#ifdef USE_NORMAL_MAPS
        tangent += R10G10B10A2Tofloat32_t4(s_VertexBuffer[index].tangent).xyz * barycentrics[i];
#endif
        vertex.texcoord += s_VertexBuffer[index].texcoord * barycentrics[i];
    }

    // ワールド座標系に変換
    vertex.position = mul(float32_t4(vertex.position, 1.0f), ObjectToWorld4x3());
    // 正規化
    normal = normalize(mul(normalize(normal), (float32_t3x3)ObjectToWorld4x3()));
#ifdef USE_NORMAL_MAPS
    tangent = normalize(mul(normalize(tangent), (float32_t3x3)ObjectToWorld4x3()));
    // 法線マップから引っ張ってくる
    vertex.normal = GetNormal(normal, tangent, vertex.texcoord);
#else
    vertex.normal = normal;
#endif

    return vertex;
}

// 入射光を取得する
float32_t3 GetIncidentColor(in float32_t3 incidentDirection, in float32_t3 position, in uint32_t recursiveCount, inout uint32_t seed) {
    // 入射方向のレイ
    RayDesc incidentRay;
    incidentRay.Origin = position;
    incidentRay.Direction = incidentDirection;
    incidentRay.TMin = 0.001f;
    incidentRay.TMax = 100000.0f;

    Payload newPayload;
    newPayload.color = float32_t3(0.0f, 0.0f, 0.0f);
    newPayload.recursiveCount = recursiveCount + 1;
    newPayload.seed = seed;

    TraceRay(
        g_TLAS, // RaytracingAccelerationStructure
        // 背面カリング
         RAY_FLAG_CULL_BACK_FACING_TRIANGLES, // RayFlags
        // レイのマスク
        RECURSIVE_MASK, // InstanceInclusionMask
        // シェーダーテーブルのオフセット
        RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // RayContributionToHitGroupIndex
        // シェーダーテーブルの係数
        MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // MultiplierForGeometryContributionToHitGroupIndex
        // ミスシェーダーのシェーダーテーブル
        MISS_SHADER_INDEX, // MissShaderIndex
        incidentRay, // Ray
        newPayload // Payload
    );
    seed = newPayload.seed;
    return newPayload.color;
}



// rateは入射前の屈折率 / 入射後の屈折率
float32_t3 Refract(in float32_t3 direction, in float32_t3 normal, in float32_t rate) {
    float32_t cosTheta1 = dot(-direction, normal);
    float32_t sin2Theta2 = rate * rate * (1.0f - cosTheta1 * cosTheta1);
    // 全反射
    if (sin2Theta2 > 1.0f) {
        return reflect(direction, normal);
    }
    float32_t cosTheta2 = sqrt(1.0f - sin2Theta2);
    return rate * direction + (rate * cosTheta1 - cosTheta2) * normal;
}

[shader("closesthit")]
void RefractionCHS(inout Payload payload, in Attributes attributes) {

    // 再帰回数が最大で光源に当たらなかった
    if (payload.recursiveCount >= MAX_RECURSIVE_COUNT) {
        payload.color = s_Material.emissive;
        return;
    }

    uint32_t meshPropertyIndex = InstanceID() + GeometryIndex();;
    InitializeMeshProperty(meshPropertyIndex);

    // レイの情報    
    float32_t3 rayOrigin = WorldRayOrigin();
    float32_t3 rayDirection = WorldRayDirection();

    // 頂点を取得
    Vertex vertex = GetVertex(attributes);

    // マテリアルを取得
    float32_t4 textureAlbedo = s_AlbedoMap.SampleLevel(g_LinearSampler, vertex.texcoord, 0);
    // アルベドテクスチャのアルファが0の場合現在のヒットを無視して次のヒットを探す
    if (textureAlbedo.a == 0.0f) {
        payload.color = GetIncidentColor(rayDirection, vertex.position + rayDirection * 0.001f, payload.recursiveCount, payload.seed);
        return;
    }
    float32_t3 albedo = s_Material.albedo * textureAlbedo.rgb;

    float32_t2 metallicRoughness = float32_t2(s_Material.metallic, s_Material.roughness) * s_MetallicRoughnessMap.SampleLevel(g_LinearSampler, vertex.texcoord, 0).zy;
    // 0が扱えないため
    PBR::Material material = PBR::CreateMaterial(albedo, metallicRoughness.x, max(metallicRoughness.y, 0.01f), s_Material.emissive);
    PBR::Geometry geometry = PBR::CreateGeometry(vertex.position, vertex.normal, rayOrigin);

    material.specularRoughness = clamp(material.specularRoughness, 0.03f, 1.0f);

    if (dot(material.emissive, float32_t3(1.0f, 1.0f, 1.0f)) >= 0.001f) {
        payload.color += material.emissive;
        return;
    }
    
    float32_t rate = 1.25f / 1.0f;
    // 空気から物体の中へ
    if (dot(rayDirection, geometry.normal) > 0.0f) {
        rate = 1.0f / 1.25f;
    } 
    float32_t3 incidentDirection = Refract(rayDirection, geometry.normal, rate);
    payload.color = GetIncidentColor(incidentDirection, vertex.position + incidentDirection * 0.001f, payload.recursiveCount, payload.seed);
}