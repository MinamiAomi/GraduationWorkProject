struct Scene {
    float4x4 orthoMatrix;
};
ConstantBuffer<Scene> scene_ : register(b0);

struct VSInput {
    float3 position : POSITION0;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input) {
    VSOutput output;

    output.position = mul(float4(input.position, 1.0f), scene_.orthoMatrix);
    output.texcoord = input.texcoord;
    output.color = input.color;
    
    return output;
}