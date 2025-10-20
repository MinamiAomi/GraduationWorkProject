#include "GeometryPass.hlsli"

struct VSInput {
    float3 position : POSITION0;
    float4 normal : NORMAL0;
    float4 tangent : TANGENT0;
    float2 texcoord : TEXCOORD0;
#ifdef ENABLE_SKINNING
    uint4 boneIndices : BLENDINDICES0;
    float4 boneWeights : BLENDWEIGHT0;
#endif
};

struct VSOutput {
    float4 svPosition : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float2 texcoord : TEXCOORD0;
    float viewDepth : TEXCOORD1;
};

float Map(float value, float min1, float max1, float min2, float max2) {
    return (value - min1) / (max1 - min1);
}

VSOutput main(VSInput input) {
    VSOutput output;
        
    float4 localPosition = float4(input.position, 1.0f);
    float3 localNormal = input.normal.xyz * 2.0f - 1.0f;
    float3 localTangent = input.tangent.xyz * 2.0f - 1.0f;
        
#ifdef ENABLE_SKINNING
    // スキニング計算

    float4x4 boneMatrix =
        mul(g_Bones[input.boneIndices.x].boneMatrix, input.boneWeights.x) +
        mul(g_Bones[input.boneIndices.y].boneMatrix, input.boneWeights.y) +
        mul(g_Bones[input.boneIndices.z].boneMatrix, input.boneWeights.z) +
        mul(g_Bones[input.boneIndices.w].boneMatrix, input.boneWeights.w);
    
    localPosition = mul(localPosition, boneMatrix);
    
    float4x4 boneMatrixInverseTranspose = 
        mul(g_Bones[input.boneIndices.x].boneInverseTransposeMatrix, input.boneWeights.x) +
        mul(g_Bones[input.boneIndices.y].boneInverseTransposeMatrix, input.boneWeights.y) +
        mul(g_Bones[input.boneIndices.z].boneInverseTransposeMatrix, input.boneWeights.z) +
        mul(g_Bones[input.boneIndices.w].boneInverseTransposeMatrix, input.boneWeights.w);
    
    localNormal = normalize(mul(localNormal, (float3x3) boneMatrixInverseTranspose));
    localTangent = normalize(mul(localTangent, (float3x3) boneMatrixInverseTranspose));
#endif
        
    float4 worldPosition = mul(localPosition, g_Instance.worldMatrix);
    float4 viewPosition = mul(worldPosition, g_Scene.viewMatrix);
    output.svPosition = mul(viewPosition, g_Scene.projectionMatrix);
    output.worldPosition = worldPosition.xyz;
    output.viewDepth = viewPosition.z;
    //output.viewDepth = (viewPosition.z - g_Scene.nearClip) / (g_Scene.farClip - g_Scene.nearClip);
    output.normal = mul(localNormal, (float3x3) g_Instance.worldInverseTransposeMatrix);
    output.tangent = mul(localTangent, (float3x3) g_Instance.worldInverseTransposeMatrix);
    output.texcoord = input.texcoord;
    
    return output;
}