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
    
    PBR::ReflectedLight reflectedLight;
    reflectedLight.directDiffuse = float32_t3(0.0f, 0.0f, 0.0f);
    reflectedLight.directSpecular = float32_t3(0.0f, 0.0f, 0.0f);
    
    for (uint32_t i = 0; i < g_Scene.lightCount; ++i) {
        Light light = g_Lights[i];
        PBR::IncidentLight incidentLight;
        float32_t attenuation = 1.0f;
        bool processLight = false;
        
        if (light.type == LIGHT_TYPE_DIRECTIONAL) {
            incidentLight.direction = -light.direction;
            incidentLight.color = light.color * light.intensity;
            attenuation = 1.0f;
            processLight = true;
        }
        else {
            float32_t3 lightVec = light.position - position;
            float32_t distance = length(lightVec);
        
            if (distance < light.range) {
                float32_t3 L = normalize(lightVec);
                incidentLight.direction = L;
                
                float32_t distRate = saturate(1.0f - distance / light.range);
                attenuation = pow(distRate, light.decay);
        
                if (light.type == LIGHT_TYPE_POINT) {
                    incidentLight.color = light.color * light.intensity;
                    processLight = true;
                }
                else if (light.type == LIGHT_TYPE_SPOT) {
                    float32_t cosAngle = dot(-L, light.direction);
                    float32_t falloffFactor = saturate((cosAngle - light.cosOuter) / (light.cosInner - light.cosOuter));
                    
                    attenuation *= falloffFactor;
                    
                    incidentLight.color = light.color * light.intensity;
                    if (falloffFactor > 0.0f) {
                        processLight = true;
                    }
                }
            }
        }
        
        if (processLight && attenuation > 0.0f) {
            incidentLight.color *= attenuation;
            PBR::DirectRenderingEquations(incidentLight, geometry, material, reflectedLight);
        }
    }

    float32_t3 ambient = material.diffuseReflectance * float32_t3(0.02f, 0.02f, 0.02f); // 少し暗めに調整

    float32_t3 color = reflectedLight.directDiffuse + reflectedLight.directSpecular + ambient;

    color = saturate(color);
    output.color.rgb = color;
    output.color.a = 1.0f;
    
    return output;
}