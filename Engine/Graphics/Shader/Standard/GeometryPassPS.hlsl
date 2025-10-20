#include "GeometryPass.hlsli"

struct PSInput {
    float4 svPosition : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float2 texcoord : TEXCOORD0;
    float viewDepth : TEXCOORD1;
};

struct PSOutput {
    float4 albedo : SV_TARGET0;
    float2 metallicRoughness : SV_TARGET1;
    float4 normal : SV_TARGET2;
    float viewDepth : SV_TARGET3;
    uint2 meshMaterialIDs : SV_TARGET4;
};

// 法線マップから法線を取得
float3 GetNormal(in float3 normal, in float3 tangent, in float2 texcoord) {
    // 法線マップからサンプリング
    float3 normalMap = g_BindlessTextures[g_Material.normalMapIndex].SampleLevel(g_Sampler, texcoord, 0).xyz;
    // UNORMからSNORMに変換
    normalMap = normalMap * 2.0f - 1.0f;
    // NormalとTangentに垂直なベクトル
    float3 binormal = normalize(cross(tangent, normal));
    // 新しい法線
    float3 newNormal = normalMap.x * tangent + normalMap.y * binormal + normalMap.z * normal;
    return newNormal;
}

PSOutput main(PSInput input) {

    PSOutput output;
    

    float4 albedo = g_BindlessTextures[g_Material.albedoMapIndex].Sample(g_Sampler, input.texcoord);
    if (albedo.a == 0.0f) {
        discard;
    }
    albedo.rgb *= g_Material.albedo;
    output.albedo.rgb = albedo.rgb;
    output.albedo.a = 1.0f;
    
    float2 metallicRoughness = g_BindlessTextures[g_Material.metallicRoughnessMapIndex].Sample(g_Sampler, input.texcoord).zy;
    metallicRoughness *= float2(g_Material.metallic, g_Material.roughness);
    output.metallicRoughness = metallicRoughness;
    
    float3 normal = GetNormal(normalize(input.normal), normalize(input.tangent), input.texcoord);
    output.normal.xyz = (normal + 1.0f) * 0.5f;
    output.normal.w = 1.0f;
  
    output.viewDepth = input.viewDepth;
    output.meshMaterialIDs = uint2(0, 0);

    return output;
}