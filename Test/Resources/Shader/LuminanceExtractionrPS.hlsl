
Texture2D<float4> src_ : register(t0);
SamplerState sampler_ : register(s0);

struct Param {
    float threshold;
    float knee;
};
ConstantBuffer<Param> param_ : register(b0);

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};

float Knee(float x, float n, float k) {
    float s = k / n * x;
    float e = (1.0f - k) / (1.0f - n) * (x - 1) + 1;
    return lerp(s, e, step(n, x));
}

PSOutput main(PSInput input) {
    PSOutput output;

    output.color = src_.Sample(sampler_, input.texcoord);

    float luminance = dot(output.color.xyz, float3(0.2125f, 0.7154f, 0.0721f));
    //clip(luminance - param_.threshold);
    output.color.xyz *= Knee(luminance, param_.threshold, param_.knee);

    return output;
}
