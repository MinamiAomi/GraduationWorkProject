

Texture2D<float4> texture_ : register(t0);
SamplerState sampler_ : register(s0);

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};
    
PSOutput main(PSInput input) {
    PSOutput output;
    
    output.color = texture_.Sample(sampler_, input.texcoord);
    
    return output;
}