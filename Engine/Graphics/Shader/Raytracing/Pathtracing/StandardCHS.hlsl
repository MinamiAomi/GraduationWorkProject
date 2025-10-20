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


float32_t3 CosineDirection(in float32_t3 normal, inout float32_t seed) {
    float32_t u = fRand(seed);
    float32_t v = fRand(seed);
    float32_t a = 6.2831853f * v;
    float32_t b = 2.0f * u - 1.0f;
    float32_t3 direction = float32_t3(sqrt(1.0f - b * b) * float32_t2(cos(a), sin(a)), b);
    return normalize(normal + direction);
}

float32_t3 LocalToWorld(in float32_t3 localDirection, in float32_t3 normal) {
    float32_t3 tangent = abs(normal.x) > 0.999f ? float32_t3(0.0f, 1.0f, 0.0f) : float32_t3(1.0f, 0.0f, 0.0f);
    tangent = normalize(cross(tangent, normal));
    float32_t3 bitangent = cross(normal, tangent);
    return localDirection.x * tangent + localDirection.y * bitangent + localDirection.z * normal;
}

float32_t3 SampleCosine(in float32_t3 normal, inout float32_t seed) {
    float32_t r1 = fRand(seed);
    float32_t r2 = fRand(seed);
    float32_t theta = acos(sqrt(1.0f - r1));
    float32_t phi = 2.0f * PI * r2;

    float32_t sinTheta = sin(theta), cosTheta = cos(theta);
    float32_t sinPhi = sin(phi), cosPhi = cos(phi);
    float32_t x = sinTheta * cosPhi;
    float32_t y = sinTheta * sinPhi;
    float32_t z = cosTheta;

    return LocalToWorld(float32_t3(x, y, z), normal);
}

float32_t PDFCosine(in float32_t3 normal, in float32_t3 direction) {
    float32_t cosTheta = dot(normal, direction);
    return cosTheta > 0.0f ? cosTheta / PI : 0.0f;
}

float32_t3 SampleGGX(in float32_t3 normal, in float32_t roughness, inout float32_t seed) {
    float32_t alpha = roughness * roughness;
    float32_t r1 = fRand(seed);
    float32_t r2 = fRand(seed);

    float32_t theta = atan(alpha * sqrt(r1) / sqrt(max(1.0f - r1, 0.0f)));
    float32_t phi = 2.0f * PI * r2;

    float32_t sinTheta = sin(theta);
    float32_t x = sinTheta * cos(phi);
    float32_t y = sinTheta * sin(phi);
    float32_t z = cos(theta);

    return LocalToWorld(float32_t3(x, y, z), normal);
}

float32_t PDFGGX(in float32_t3 halfV, in float32_t3 normal, in float32_t roughness) { 
    float32_t alpha = roughness * roughness;
    float32_t cosTheta = dot(halfV, normal);
    if (cosTheta <= 0.0f) { return 0.0f; }
    float32_t alpha2 = alpha * alpha;
    float32_t t = (cosTheta * cosTheta) * (alpha2 - 1.0f) + 1.0f;
    float32_t D = alpha2 / (PI * t * t);
    return D * cosTheta;
}


[shader("closesthit")]
void StandardCHS(inout Payload payload, in Attributes attributes) {

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

    // ランダムな半球状のベクトル
    //float32_t3 incidentDirection = CosineDirection(geometry.normal, payload.seed);
    float32_t3 incidentDirection = SampleCosine(geometry.normal, payload.seed);
    //float32_t3 incidentDirection = SampleGGX(geometry.normal, material.specularRoughness, payload.seed);

    float32_t3 brdf =
        PBR::DiffuseBRDF(material.diffuseReflectance) +
        PBR::SpecularBRDF(incidentDirection, geometry.normal, geometry.viewDirection, material.specularReflectance, material.specularRoughness);
    // 入射光
    float32_t3 incidentColor = GetIncidentColor(incidentDirection, vertex.position + vertex.normal * 0.001f, payload.recursiveCount, payload.seed);
    // 確率密度関数
    //float32_t pdf = 1.0f / (2.0f * PI);
    float32_t pdf = PDFCosine(geometry.normal, incidentDirection);
    //float32_t pdf = PDFGGX(normalize(incidentDirection + geometry.viewDirection), geometry.normal, material.specularRoughness);
    // コサイン項
    float32_t cosine = saturate(dot(incidentDirection, vertex.normal));
    payload.color += incidentColor * brdf * cosine / (pdf + EPSILON);
}