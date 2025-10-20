#include "StandardModel.hlsli"

struct VSInput {
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

struct VSOutput {
    float4 svPosition : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input) {
    VSOutput output;
        
    float4 localPosition = float4(input.position, 1.0f);
    float3 localNormal = input.normal;
    
    float4 worldPosition = mul(localPosition, g_Instance.worldMatrix);
    output.svPosition = mul(worldPosition, mul(g_Scene.viewMatrix, g_Scene.projectionMatrix));
    output.worldPosition = worldPosition.xyz;
    output.normal = mul(localNormal, (float3x3) g_Instance.worldInverseTransposeMatrix);
    output.texcoord = input.texcoord;
    
    return output;
}