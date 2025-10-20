#include "../Lighting.hlsli"

#define TRUE_UINT 1
#define FALSE_UINT 0

#define INVALID_COLOR float3(-1.0f, -1.0f, -1.0f)


//////////////////////////////////////////////////
// R + M * G + I HitGroupShaderRecordIndex      | R = RayContributionToHitGroupIndex
// 0 + 0 * G + 0 ShadowHitGroup                 | M = MultiplierForGeometryContributionToHitGroupIndex
// 1 + 2 * G + I PrimaryHitGroup                | G = GeometryContributionToHitGroupIndex
// 2 + 2 * G + I ReflectionHitGroup             | I = InstanceContributionToHitGroupIndex
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// 1 + 2 * G + I PrimaryHitGroup
#define PRIMARY_RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX 1
#define PRIMARY_MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX 2
#define PRIMARY_MISS_SHADER_INDEX 0
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// 0 + 0 * G + 0 ShadowHitGroup
#define SHADOW_RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX 0
#define SHADOW_MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX 0
#define SHADOW_MISS_SHADER_INDEX 1
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// 2 + 2 * G + I ReflectionHitGroup
#define REFLECTION_RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX 2
#define REFLECTION_MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX 2
#define REFLECTION_MISS_SHADER_INDEX 2
//////////////////////////////////////////////////

#define PRIMARY_RAY_ATTRIBUTE (1 << 0)
#define SHADOW_RAY_ATTRIBUTE  (1 << 1)


// シーン
struct Scene {
    float4x4 viewProjectionInverseMatrix;
    float3 cameraPosition;
    float3 sunLightDirection;
    float sunLightIntensity;
    float3 sunLightColor;
};

// 一次レイ用ペイロード
struct PrimaryPayload {
    float shadow;
    float3 reflection;
};
// シャドウレイ用ペイロード
struct ShadowPayload {
    uint isHit;
};
// 反射レイ用ペイロード
struct ReflectionPayload {
    uint isHit;
    float3 color;
};

struct Attributes {
    float2 barycentrics;
};

ConstantBuffer<Scene> g_Scene : register(b0);

RaytracingAccelerationStructure g_TLAS : register(t0);
RaytracingAccelerationStructure g_CastShadowTLAS : register(t1);
RWTexture2D<float4> g_Shadow : register(u0);
RWTexture2D<float4> g_Reflection : register(u1);


//////////////////////////////////////////////////


// texcoodとdepthからワールド座標を計算
float3 GetWorldPosition(in float2 texcoord, in float depth, in float4x4 viewProjectionInverseMatrix) {
    // xは0~1から-1~1, yは0~1から1~-1に上下反転
    float2 xy = texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float4 position = float4(xy, depth, 1.0f);
    position = mul(position, viewProjectionInverseMatrix);
    position.xyz /= position.w;
    return position.xyz;
}

[shader("raygeneration")]
void RayGeneration() {
    // レイのインデックス
    uint2 dispatchRaysIndex = DispatchRaysIndex().xy;
    // レイの縦横数
    uint2 dispatchRaysDimensions = DispatchRaysDimensions().xy;
    // テクスチャ座標系を求める    
    float2 texcoord = ((float2) dispatchRaysIndex + 0.5f) / (float2) dispatchRaysDimensions;
    
    //ConstantBuffer<Scene> scene = ResourceDescriptorHeap[descriptorIndex.scene];
    // 近面から遠面へのレイ
    RayDesc primaryRay;
    float3 rayOrigin = GetWorldPosition(texcoord, 0.0f, g_Scene.viewProjectionInverseMatrix);
    primaryRay.Origin = rayOrigin;
    float3 rayDiff = GetWorldPosition(texcoord, 1.0f, g_Scene.viewProjectionInverseMatrix) - rayOrigin;
    primaryRay.Direction = normalize(rayDiff);
    primaryRay.TMin = 0.0f;
    primaryRay.TMax = length(rayDiff);
    // レイを飛ばす
    PrimaryPayload primaryPayload;
    primaryPayload.shadow = 1.0f;
    primaryPayload.reflection = float3(0.0f, 0.0f, 0.0f);
    TraceRay(
        g_TLAS, // RaytracingAccelerationStructure
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES, // RayFlags
        PRIMARY_RAY_ATTRIBUTE, // InstanceInclusionMask
        PRIMARY_RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // RayContributionToHitGroupIndex
        PRIMARY_MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // MultiplierForGeometryContributionToHitGroupIndex
        PRIMARY_MISS_SHADER_INDEX, // MissShaderIndex
        primaryRay, // Ray
        primaryPayload); // Payload

    float shadow = primaryPayload.shadow;
    g_Shadow[dispatchRaysIndex].rgb = shadow;
    g_Shadow[dispatchRaysIndex].a = 1.0f;
    
    g_Reflection[dispatchRaysIndex].rgb = primaryPayload.reflection;
    g_Reflection[dispatchRaysIndex].a = 1.0f;
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////


struct Vertex {
    float3 position;
    float3 normal;
    float2 texcoord;
};

struct Material {
    float3 color;
    uint useLighting;
    float3 diffuse;
    float shininess;
    float3 specular;
};

StructuredBuffer<Vertex> l_VertexBuffer : register(t0, space1);
StructuredBuffer<uint> l_IndexBuffer : register(t1, space1);
Texture2D<float4> l_Texture : register(t2, space1);
SamplerState l_Sampler : register(s0, space1);
ConstantBuffer<Material> l_Material : register(b0, space1);

float3 CalcBarycentrics(float2 barycentrics) {
    return float3(1.0f - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);
}
    
Vertex GetVertex(Attributes attributes) {
    Vertex vertex = (Vertex) 0;
    float3 barycentrics = CalcBarycentrics(attributes.barycentrics);
    uint primitiveID = PrimitiveIndex() * 3;

    for (uint i = 0; i < 3; ++i) {
        uint index = l_IndexBuffer[primitiveID + i];
        vertex.position += l_VertexBuffer[index].position * barycentrics[i];
        vertex.normal += l_VertexBuffer[index].normal * barycentrics[i];
        vertex.texcoord += l_VertexBuffer[index].texcoord * barycentrics[i];
    }
    vertex.normal = normalize(vertex.normal);
    vertex.position = mul(float4(vertex.position, 1.0f), ObjectToWorld4x3());
    vertex.normal = mul(vertex.normal, (float3x3) ObjectToWorld4x3());
    
    return vertex;
}
    
[shader("closesthit")]
void PrimaryClosestHit(inout PrimaryPayload payload, in Attributes attributes) {

    // レイの情報    
    float hitT = RayTCurrent();
    float3 rayOrigin = WorldRayOrigin();
    float3 rayDirection = WorldRayDirection();
    // 頂点を取得
    Vertex vertex = GetVertex(attributes);
    
    // 影
    bool reciveShadow = (InstanceID() == 1);
    if (reciveShadow) {
        // 衝突点からライトへのレイ
        RayDesc shadowRay;
        // 発射点を法線方向に少しずらす
        shadowRay.Origin = vertex.position;
        shadowRay.Direction = -g_Scene.sunLightDirection;
        shadowRay.TMin = 0.001f; // 少し浮かす
        shadowRay.TMax = 100000.0f; // 
        
        ShadowPayload shadowPayload;
        shadowPayload.isHit = 0;
        
        TraceRay(
            g_CastShadowTLAS,
            RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
            SHADOW_RAY_ATTRIBUTE,
            SHADOW_RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // RayContributionToHitGroupIndex
            SHADOW_MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // MultiplierForGeometryContributionToHitGroupIndex
            SHADOW_MISS_SHADER_INDEX, // MissShaderIndex
            shadowRay,
            shadowPayload);
        
        payload.shadow = lerp(1.0f, 0.5f, shadowPayload.isHit);
    }

    if (l_Material.useLighting == 0) {
        return;
    }
        
    // 反射したレイの方向
    float3 reflectionRayDirection = reflect(rayDirection, vertex.normal);
    RayDesc reflectionRay;
    reflectionRay.Origin = vertex.position;
    reflectionRay.Direction = normalize(reflectionRayDirection);
    reflectionRay.TMin = 0.0001f;
    reflectionRay.TMax = 10000.0f;
    
    ReflectionPayload reflectionPayload;
    TraceRay(
        g_TLAS, // RaytracingAccelerationStructure
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES, // RayFlags
        PRIMARY_RAY_ATTRIBUTE, // InstanceInclusionMask
        REFLECTION_RAY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // RayContributionToHitGroupIndex
        REFLECTION_MULTIPLIER_FOR_GEOMETRY_CONTRIBUTION_TO_HIT_GROUP_INDEX, // MultiplierForGeometryContributionToHitGroupIndex
        REFLECTION_MISS_SHADER_INDEX, // MissShaderIndex
        reflectionRay, // Ray
        reflectionPayload); // Payload
    
    float3 specular = l_Material.specular;
    specular *= Lighting::PhongReflection(vertex.normal, WorldRayDirection(), g_Scene.sunLightDirection, l_Material.shininess);
    specular *= (reflectionPayload.isHit == 1) ? reflectionPayload.color : g_Scene.sunLightColor * g_Scene.sunLightIntensity;
    payload.reflection = specular;
   // payload.reflection = (reflectionPayload.isHit == 1) ? reflectionPayload.color : g_Scene.sunLightColor * g_Scene.sunLightIntensity;
}


//////////////////////////////////////////////////


[shader("miss")]
void PrimaryMiss(inout PrimaryPayload payload) {
    payload.shadow = 1.0f;
    payload.reflection = float3(0.0f, 0.0f, 0.0f);
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////


[shader("closesthit")]
void ShadowClosestHit(inout ShadowPayload payload, in Attributes attributes) {
    payload.isHit = TRUE_UINT;
}


//////////////////////////////////////////////////


[shader("miss")]
void ShadowMiss(inout ShadowPayload payload) {
    payload.isHit = FALSE_UINT;
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

// 反射レイ
[shader("closesthit")]
void ReflectionClosestHit(inout ReflectionPayload payload, in Attributes attributes) {
    payload.isHit = TRUE_UINT;
    // 頂点
    Vertex vertex = GetVertex(attributes);
    if (l_Material.useLighting == 1) {
        // 拡散反射
        float3 diffuse = l_Material.diffuse;
        diffuse *= l_Material.color;
        diffuse *= l_Texture.SampleLevel(l_Sampler, vertex.texcoord, 0).rgb;
        diffuse *= Lighting::HalfLambertReflection(vertex.normal, g_Scene.sunLightDirection);
        // 鏡面反射
        float3 specular = l_Material.specular;
        specular *= Lighting::PhongReflection(vertex.normal, WorldRayDirection(), g_Scene.sunLightDirection, l_Material.shininess);
    
        // シェーディングによる色
        payload.color = (diffuse + specular) * g_Scene.sunLightColor * g_Scene.sunLightIntensity;
    }
    else {
        payload.color = l_Texture.SampleLevel(l_Sampler, vertex.texcoord, 0).rgb * l_Material.color;
    }
}

//////////////////////////////////////////////////

[shader("miss")]
void ReflectionMiss(inout ReflectionPayload payload) {
    payload.isHit = FALSE_UINT;
    payload.color = float3(0.0f, 0.0f, 0.0f);
}