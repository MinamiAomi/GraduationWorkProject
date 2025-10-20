struct Scene {
    float4x4 viewProjMatrix;
};
ConstantBuffer<Scene> scene_ : register(b0);

struct Instance {
    float4x4 worldMatrix;
    float3 color;
    float alpha;
};
StructuredBuffer<Instance> instances_ : register(t0);

struct VSInput {
    float3 position : POSITION0;
    uint instanceID : SV_INSTANCEID;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    uint instanceID : SV_INSTANCEID;
};

VSOutput main(VSInput input) {
    VSOutput output;

    float4 position = mul(float4(input.position, 1.0f), instances_[input.instanceID].worldMatrix);
    
    output.position = mul(position, scene_.viewProjMatrix);
    output.texcoord = input.position.xy;
    output.instanceID = input.instanceID;
    
    return output;
}