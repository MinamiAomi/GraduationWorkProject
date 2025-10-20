#include "LightingPass.hlsli"
#include "../PBR.hlsli"

struct PSInput {
    float32_t4 svPosition : SV_POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float32_t4 color : SV_TARGET0;
};

float32_t3 GetWorldPosition(in float32_t2 texcoord) {
    // 深度をサンプリング
    float32_t depth = g_Depth.SampleLevel(g_DefaultSampler, texcoord, 0);
    // xは0~1から-1~1, yは0~1から1~-1に上下反転
    float32_t2 xy = texcoord * float32_t2(2.0f, -2.0f) + float32_t2(-1.0f, 1.0f);
    float32_t4 tmpPosition = float32_t4(xy, depth, 1.0f);
    tmpPosition = mul(tmpPosition, g_Scene.viewProjectionInverseMatrix);
    return tmpPosition.xyz / tmpPosition.w;
}

// 拡散反射IBL
float32_t3 DiffuseIBL(float32_t3 normal, float32_t3 diffuseReflectance) {
    return diffuseReflectance * g_Irradiance.Sample(g_CubeMapSampler, normal);
}

// 鏡面反射IBL
float32_t3 SpecularIBL(float32_t3 normal, float32_t3 viewDirection, float32_t3 specularReflectance, float32_t specularRoughness) {
    float32_t NdotV = saturate(dot(normal, viewDirection));
    float32_t lod = specularRoughness * (g_Scene.radianceMipCount);
    float32_t3 specular = PBR::SchlickFresnel(specularReflectance, 1.0f, NdotV);
    return specular * g_Radiance.SampleLevel(g_CubeMapSampler, reflect(-viewDirection, normal), lod);
}

PSOutput main(PSInput input) {

    PSOutput output;
    
    //InitializeSurfaceProperties(input);
    
    // AlbedoのWが0の場合は計算しない
    if (g_Albedo.SampleLevel(g_DefaultSampler, input.texcoord, 0).w == 0.0f) {
        float32_t3 position = GetWorldPosition(input.texcoord);

        output.color.rgb = AtmosphericScattering(g_Scene.cameraPosition, normalize(position - g_Scene.cameraPosition), g_SkyParameter).rgb;
        output.color.a = 1.0f;
        return output;
    }
   
    float32_t3 position = GetWorldPosition(input.texcoord);
    float32_t3 normal = g_Normal.SampleLevel(g_DefaultSampler, input.texcoord, 0) * 2.0f - 1.0f;
    float32_t3 albedo = g_Albedo.SampleLevel(g_DefaultSampler, input.texcoord, 0).xyz;
    float32_t metallic = g_MetallicRoughness.SampleLevel(g_DefaultSampler, input.texcoord, 0).x;
    float32_t roughness = g_MetallicRoughness.SampleLevel(g_DefaultSampler, input.texcoord, 0).y;
    // 0はダメ
    //roughness = clamp(roughness, 0.03f, 1.0f);
    float32_t3 emissive = float32_t3(0.0f, 0.0f, 0.0f);

    PBR::Geometry geometry = PBR::CreateGeometry(position, normal, g_Scene.cameraPosition);
    PBR::Material material = PBR::CreateMaterial(albedo, metallic, roughness, emissive);
    PBR::IncidentLight incidentLight;
    incidentLight.direction = g_SkyParameter.sunPosition;
    incidentLight.color = float32_t3(1.0f, 1.0f, 1.0f);
    
    PBR::ReflectedLight reflectedLight;
    reflectedLight.directDiffuse = float32_t3(0.0f, 0.0f, 0.0f);
    reflectedLight.directSpecular = float32_t3(0.0f, 0.0f, 0.0f);
    
    ///color += ShadeDirectionalLight(g_Scene.directionalLight);
    PBR::DirectRenderingEquations(incidentLight, geometry, material, reflectedLight);
    
    // IBL
    //reflectedLight.directDiffuse += DiffuseIBL(geometry.normal, material.diffuseReflectance);
    //reflectedLight.directSpecular += SpecularIBL(geometry.normal, geometry.viewDirection, material.specularReflectance, material.specularRoughness);

    float32_t3 color = reflectedLight.directDiffuse + reflectedLight.directSpecular;

    output.color.rgb = color;
    output.color.a = 1.0f;
    
    return output;
}