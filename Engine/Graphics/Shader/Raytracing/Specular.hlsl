#include "../PBR.hlsli"

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
#define MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX 1
#define MISS_SHADER_INDEX 0
//////////////////////////////////////////////////


#define RAY_ATTRIBUTE (1 << 0)

static const uint32_t MAX_RECURSIVE_COUNT = 3;

// シーン
struct Scene {
    float32_t4x4 viewProjectionInverseMatrix;
    float32_t3 cameraPosition;
};

// ペイロード
struct Payload {
    float32_t3 color;
    uint32_t recursiveCount;    // 再帰回数
};

struct Attributes {
    float32_t2 barycentrics;
};

ConstantBuffer<Scene> g_Scene : register(b0);

RaytracingAccelerationStructure g_TLAS : register(t0);
TextureCube<float32_t4> g_Skybox : register(t1);
RWTexture2D<float32_t4> g_Color : register(u0);
SamplerState g_PointSampler : register(s0);
SamplerState g_LinearSampler : register(s1);

Texture2D<float32_t4> g_BindlessTextures[] : register(t0, space2);

//////////////////////////////////////////////////

float3 LinearToSRGB(float3 color) {
    float3 sqrt1 = sqrt(color);
    float3 sqrt2 = sqrt(sqrt1);
    float3 sqrt3 = sqrt(sqrt2);
    float3 srgb = 0.662002687 * sqrt1 + 0.684122060 * sqrt2 - 0.323583601 * sqrt3 - 0.0225411470 * color;
    return srgb;
}

// texcoodとdepthからワールド座標を計算
float32_t3 GetWorldPosition(in float32_t2 texcoord, in float32_t depth, in float32_t4x4 viewProjectionInverseMatrix) {
    // xは0~1から-1~1, yは0~1から1~-1に上下反転
    float32_t2 xy = texcoord * float32_t2(2.0f, -2.0f) + float32_t2(-1.0f, 1.0f);
    float32_t4 position = float32_t4(xy, depth, 1.0f);
    position = mul(position, viewProjectionInverseMatrix);
    position.xyz /= position.w;
    return position.xyz;
}

[shader("raygeneration")]
void RayGeneration() {
    // レイのインデックス
    uint32_t2 dispatchRaysIndex = DispatchRaysIndex().xy;
    // レイの縦横数
    uint32_t2 dispatchRaysDimensions = DispatchRaysDimensions().xy;
    // テクスチャ座標系を求める    
    float32_t2 texcoord = ((float32_t2) dispatchRaysIndex + 0.5f) / (float32_t2)dispatchRaysDimensions;

    // 近面から遠面へのレイ
    RayDesc ray;
    float32_t3 rayOrigin = GetWorldPosition(texcoord, 0.0f, g_Scene.viewProjectionInverseMatrix);
    ray.Origin = rayOrigin;
    float32_t3 rayDiff = GetWorldPosition(texcoord, 1.0f, g_Scene.viewProjectionInverseMatrix) - rayOrigin;
    ray.Direction = normalize(rayDiff);
    ray.TMin = 0.0f;
    ray.TMax = length(rayDiff);
    // レイを飛ばす
    Payload payload;
    payload.color = float32_t3(0.0f, 0.0f, 0.0f);
    payload.recursiveCount = 0; // 再帰回数 0
    TraceRay(
        g_TLAS, // RaytracingAccelerationStructure
        // 背面カリング
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES, // RayFlags
        // レイのマスク
        RAY_ATTRIBUTE, // InstanceInclusionMask
        // シェーダーテーブルのオフセット
        RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // RayContributionToHitGroupIndex
        // シェーダーテーブルの係数
        MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // MultiplierForGeometryContributionToHitGroupIndex
        // ミスシェーダーのシェーダーテーブル
        MISS_SHADER_INDEX, // MissShaderIndex
        ray, // Ray
        payload // Payload
    );

    g_Color[dispatchRaysIndex].rgb = payload.color;
    //g_Color[dispatchRaysIndex].rgb = float32_t3(1.0f, 1.0f, 1.0f);
    g_Color[dispatchRaysIndex].a = 1.0f;
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

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
    float32_t roughness;
    uint32_t albedoMapIndex;
    uint32_t metallicRoughnessMapIndex;
    uint32_t normalMapIndex;
};

StructuredBuffer<PackedVertex> l_VertexBuffer : register(t0, space1);
StructuredBuffer<uint32_t> l_IndexBuffer : register(t1, space1);
ConstantBuffer<Material> l_Material : register(b0, space1);

float32_t3 CalcBarycentrics(float32_t2 barycentrics) {
    return float32_t3(1.0f - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);
}

float32_t4 R10G10B10A2Tofloat32_t4(uint32_t value) {
    float32_t x = (float32_t)((value >> 0) & 0x3FF) / 1023.0f;
    float32_t y = (float32_t)((value >> 10) & 0x3FF) / 1023.0f;
    float32_t z = (float32_t)((value >> 20) & 0x3FF) / 1023.0f;
    float32_t w = (float32_t)((value >> 30) & 0x3) / 1023.0f;
    return float32_t4(x, y, z, w) * 2.0f - 1.0f;
}
// 法線を計算
float32_t3 GetNormal(in float32_t3 normal, in float32_t3 tangent, in float32_t2 texcoord) {
    // 法線マップからサンプリング
    float32_t3 normalMap = g_BindlessTextures[l_Material.normalMapIndex].SampleLevel(g_PointSampler, texcoord, 0).xyz;
    // UNORMからSNORMに変換
    normalMap = normalMap * 2.0f - 1.0f;
    // NormalとTangentに垂直なベクトル
    float32_t3 binormal = normalize(cross(tangent, normal));
    // 新しい法線
    float32_t3 newNormal = normalMap.x * tangent + normalMap.y * binormal + normalMap.z * normal;
    return newNormal;
}
// ワールド座標の頂点を計算
Vertex GetVertex(Attributes attributes) {
    Vertex vertex = (Vertex)0;
    float32_t3 barycentrics = CalcBarycentrics(attributes.barycentrics);
    uint32_t primitiveID = PrimitiveIndex() * 3;

    float32_t3 normal = float32_t3(0.0f, 0.0f, 0.0f), tangent = float32_t3(0.0f, 0.0f, 0.0f);
    for (uint32_t i = 0; i < 3; ++i) {
        uint32_t index = l_IndexBuffer[primitiveID + i];
        vertex.position += l_VertexBuffer[index].position * barycentrics[i];
        normal += R10G10B10A2Tofloat32_t4(l_VertexBuffer[index].normal).xyz * barycentrics[i];
#ifdef USE_NORMAL_MAPS
        tangent += R10G10B10A2Tofloat32_t4(l_VertexBuffer[index].tangent).xyz * barycentrics[i];
#endif
        vertex.texcoord += l_VertexBuffer[index].texcoord * barycentrics[i];
    }
    
    // ワールド座標系に変換
    vertex.position = mul(float32_t4(vertex.position, 1.0f), ObjectToWorld4x3());
    // 正規化
    normal = normalize(mul(normalize(normal), (float32_t3x3) ObjectToWorld4x3()));
#ifdef USE_NORMAL_MAPS
    tangent = normalize(mul(normalize(tangent), (float32_t3x3) ObjectToWorld4x3()));
    // 法線マップから引っ張ってくる
    vertex.normal = GetNormal(normal, tangent, vertex.texcoord);
#else
    vertex.normal = normal;
#endif

    return vertex;
}

[shader("closesthit")]
void RecursiveClosestHit(inout Payload payload, in Attributes attributes) {

    // 再帰回数が最大で光源に当たらなかった
    if (payload.recursiveCount >= MAX_RECURSIVE_COUNT) {
        payload.color = float32_t3(0.0f, 0.0f, 0.0f);
        return;
    }

    // レイの情報    
    float32_t hitT = RayTCurrent();
    float32_t3 rayOrigin = WorldRayOrigin();
    float32_t3 rayDirection = WorldRayDirection();
    // 頂点を取得
    Vertex vertex = GetVertex(attributes);
    payload.color = vertex.normal;

    float32_t3 incidentDirection = normalize(reflect(rayDirection, vertex.normal));
    // 入射方向のレイ
    RayDesc incidentRay;
    incidentRay.Origin = vertex.position;
    incidentRay.Direction = incidentDirection;
    incidentRay.TMin = 0.001f;
    incidentRay.TMax = 100000.0f;

    // 再帰回数を増やす
    ++payload.recursiveCount;    

    TraceRay(
        g_TLAS, // RaytracingAccelerationStructure
        // 背面カリング
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES, // RayFlags
        // レイのマスク
        RAY_ATTRIBUTE, // InstanceInclusionMask
        // シェーダーテーブルのオフセット
        RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // RayContributionToHitGroupIndex
        // シェーダーテーブルの係数
        MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // MultiplierForGeometryContributionToHitGroupIndex
        // ミスシェーダーのシェーダーテーブル
        MISS_SHADER_INDEX, // MissShaderIndex
        incidentRay, // Ray
        payload // Payload
    );

    float32_t3 albedo = l_Material.albedo * g_BindlessTextures[l_Material.albedoMapIndex].SampleLevel(g_LinearSampler, vertex.texcoord, 0).rgb;
    float32_t2 metallicRoughness = float32_t2(l_Material.metallic, l_Material.roughness) * g_BindlessTextures[l_Material.metallicRoughnessMapIndex].SampleLevel(g_LinearSampler, vertex.texcoord, 0).zy;
    // 0が扱えないため
    metallicRoughness.y = clamp(metallicRoughness.y, 0.03f, 1.0f);
    PBR::Material material = PBR::CreateMaterial(albedo, metallicRoughness.x, metallicRoughness.y);
    PBR::Geometry geometry = PBR::CreateGeometry(vertex.position, vertex.normal, rayOrigin);
    PBR::IncidentLight incidentLight;
    incidentLight.direction = incidentDirection;
    incidentLight.color = payload.color * 3.14159265359f;

    float32_t3 irradiance = incidentLight.color * saturate(dot(geometry.normal, incidentLight.direction));
    float32_t3 brdf = PBR::SpecularBRDF(incidentLight.direction, geometry.normal, geometry.viewDirection, material.specularReflectance, material.specularRoughness);
    if (brdf.r >= 1.0f) {
        payload.color = float32_t3(1.0f, 0.0f, 0.0f);
        return;
    }
    payload.color = PBR::SpecularBRDF(incidentLight.direction, geometry.normal, geometry.viewDirection, material.specularReflectance, material.specularRoughness);

}


//////////////////////////////////////////////////


[shader("miss")]
void RecursiveMiss(inout Payload payload) {
    float32_t3 rayDirection = WorldRayDirection();
    payload.color = g_Skybox.SampleLevel(g_LinearSampler, rayDirection, 0).rgb;
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////