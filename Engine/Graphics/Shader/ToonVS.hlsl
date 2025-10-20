struct Scene {
    float4x4 viewProjMatrix;
    float3 cameraPosition;
};
ConstantBuffer<Scene> scene_ : register(b0);

struct Instance {
    float4x4 worldMatrix;
};
ConstantBuffer<Instance> instance_ : register(b1);

struct VSInput {
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 normal : NORMAL0;
    float2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.worldPosition = mul(float4(input.position.xyz, 1.0f), instance_.worldMatrix).xyz;
    output.position = mul(float4(output.worldPosition, 1.0f), scene_.viewProjMatrix);
    output.normal = mul(input.normal, (float3x3)instance_.worldMatrix);
    output.texcoord = input.texcoord;
    return output;
}