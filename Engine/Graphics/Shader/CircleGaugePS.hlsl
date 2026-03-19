Texture2D<float4> texture_ : register(t0);
SamplerState sampler_ : register(s0);

struct ExtraParam {
    float progress;
    float3 padding;
};
ConstantBuffer<ExtraParam> extraParam : register(b1);

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};
    
struct PSOutput {
    float4 color : SV_TARGET0;
};


float3 LinearToSRGB(float3 color) {
    float3 sqrt1 = sqrt(color);
    float3 sqrt2 = sqrt(sqrt1);
    float3 sqrt3 = sqrt(sqrt2);
    float3 srgb = 0.662002687 * sqrt1 + 0.684122060 * sqrt2 - 0.323583601 * sqrt3 - 0.0225411470 * color;
    return srgb;
}

PSOutput main(PSInput input) {
    PSOutput output;

    float2 uv = input.texcoord - float2(0.5, 0.5);
    
    float angle = atan2(uv.x, -uv.y);
    
    const float PI = 3.1415926535f;
    const float TWO_PI = 6.283185307f;
    if (angle < 0) {
        angle += TWO_PI;
    }
    
    float threshold = extraParam.progress * TWO_PI;
    
    if (angle > threshold) {
        discard;
    }
    
    float4 texColor = texture_.Sample(sampler_, input.texcoord);
    output.color = input.color * texColor;
    
    output.color.rgb = LinearToSRGB(output.color.rgb); 

    return output;
}