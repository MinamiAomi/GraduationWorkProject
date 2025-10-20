#include "StandardModel.hlsli"
#include "../Lighting.hlsli"


struct PSInput {
    float4 svPosition : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};

PSOutput main(PSInput input) {
    // 位置
    float3 position = input.worldPosition;
    // 法線
    float3 normal = normalize(input.normal);
    // ピクセルからカメラへのベクトル 
    float3 pixelToCamera = normalize(g_Scene.cameraPosition - position);
      
    
    // テクスチャの色
    float4 textureColor = g_Texture.Sample(g_Sampler, input.texcoord);
    // 拡散反射
    float3 diffuse = g_Material.diffuse * Lighting::HalfLambertReflection(normal, g_Scene.sunLightDirection);
    // 鏡面反射
    //float3 specular = g_Material.specular * Lighting::BlinnPhongReflection(normal, pixelToCamera, g_Scene.sunLightDirection, g_Material.shininess);
    // シェーディングによる色
    float3 shadeColor = diffuse * g_Scene.sunLightColor * g_Scene.sunLightIntensity;
    
    // ライティングを使用しない場合テクスチャの色をそのまま使う
    shadeColor = lerp(float3(1.0f, 1.0f, 1.0f), shadeColor, float(g_Instance.useLighting));
       
    PSOutput output;
    output.color.rgb = g_Instance.color * textureColor.rgb * shadeColor;
    output.color.a = g_Instance.alpha * textureColor.a;

    if (output.color.a <= 0.0f) {
        discard;
    }
    
    return output;
}