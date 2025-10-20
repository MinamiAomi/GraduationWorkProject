struct Param {
    float4 weight[2];
};
ConstantBuffer<Param> param_ : register(b0);

Texture2D<float4> src_ : register(t0);
SamplerState sampler_ : register(s0);

struct PSInput {
    float4 position : SV_POSITION;
    float4 texcoord[8] : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};

PSOutput main(PSInput input) {
    PSOutput output;
    
    output.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    output.color += param_.weight[0].x * src_.Sample(sampler_, input.texcoord[0].xy);
    output.color += param_.weight[0].y * src_.Sample(sampler_, input.texcoord[1].xy);
    output.color += param_.weight[0].z * src_.Sample(sampler_, input.texcoord[2].xy);
    output.color += param_.weight[0].w * src_.Sample(sampler_, input.texcoord[3].xy);
    output.color += param_.weight[1].x * src_.Sample(sampler_, input.texcoord[4].xy);
    output.color += param_.weight[1].y * src_.Sample(sampler_, input.texcoord[5].xy);
    output.color += param_.weight[1].z * src_.Sample(sampler_, input.texcoord[6].xy);
    output.color += param_.weight[1].w * src_.Sample(sampler_, input.texcoord[7].xy);
    
    output.color += param_.weight[0].x * src_.Sample(sampler_, input.texcoord[0].zw);
    output.color += param_.weight[0].y * src_.Sample(sampler_, input.texcoord[1].zw);
    output.color += param_.weight[0].z * src_.Sample(sampler_, input.texcoord[2].zw);
    output.color += param_.weight[0].w * src_.Sample(sampler_, input.texcoord[3].zw);
    output.color += param_.weight[1].x * src_.Sample(sampler_, input.texcoord[4].zw);
    output.color += param_.weight[1].y * src_.Sample(sampler_, input.texcoord[5].zw);
    output.color += param_.weight[1].z * src_.Sample(sampler_, input.texcoord[6].zw);
    output.color += param_.weight[1].w * src_.Sample(sampler_, input.texcoord[7].zw);
    
    output.color.w = 1.0f;
    
    return output;
}
