struct Instance {
    float4x4 worldMatrix;
    float3 color;
    float alpha;
    float outlineWidth;
    float3 outlineColor;
};
ConstantBuffer<Instance> instance_ : register(b1);

struct PSOutput {
    float4 color : SV_TARGET0;
};

PSOutput main() {
    PSOutput output;

    output.color = float4(instance_.outlineColor, 1.0f);
    
    return output;
}