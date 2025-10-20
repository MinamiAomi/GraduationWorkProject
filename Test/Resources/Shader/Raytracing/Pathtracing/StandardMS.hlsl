#include "Pathtracing.hlsli"
#include "../../AtmosphericScattering.hlsli"

ConstantBuffer<SkyParameter> l_SkyParameter : register(b0, space3);

[shader("miss")]
void StandardMS(inout Payload payload) {
    float32_t3 rayDirection = WorldRayDirection();
    float32_t3 rayOrigin = WorldRayOrigin();
        
    // 大気散乱を計算する
    payload.color = AtmosphericScattering(rayOrigin, normalize(rayDirection), l_SkyParameter).rgb;
}     