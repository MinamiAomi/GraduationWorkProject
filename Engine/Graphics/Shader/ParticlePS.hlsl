struct Instance {
    float4x4 worldMatrix;
    float3 color;
    float alpha;
};
StructuredBuffer<Instance> instances_ : register(t0);

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    uint instanceID : SV_INSTANCEID;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};

PSOutput main(PSInput input) {
    PSOutput output;

    float alpha = 1.0f - length(input.texcoord);
    if (alpha < 0.0f) {
        discard;
    }
    output.color = float4(instances_[input.instanceID].color, instances_[input.instanceID].alpha * alpha);
    
    return output;
}