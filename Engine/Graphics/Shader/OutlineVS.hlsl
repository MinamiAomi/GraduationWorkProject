struct Scene {
    float4x4 viewProjMatrix;
    float3 cameraPosition;
};
ConstantBuffer<Scene> scene_ : register(b0);

struct Instance {
    float4x4 worldMatrix;
    float3 color;
    float alpha;
    float outlineWidth;
    float3 outlineColor;
};
ConstantBuffer<Instance> instance_ : register(b1);

struct VSInput {
    float3 position : POSITION0;
    float3 normal : NORMAL0;
};

struct VSOutput {
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input) {
    VSOutput output;

    float3 worldNormal = mul(input.normal, (float3x3) instance_.worldMatrix);
    float3 worldPosition = mul(float4(input.position, 1.0f), instance_.worldMatrix).xyz;
    worldPosition += normalize(worldNormal) * instance_.outlineWidth;
    
    output.position = mul(float4(worldPosition, 1.0f), scene_.viewProjMatrix);
    
    return output;
}