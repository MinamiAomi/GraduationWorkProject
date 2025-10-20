
#define NUM_TEXTURES 4

Texture2D<float4> blurTexture0_ : register(t0);
Texture2D<float4> blurTexture1_ : register(t1);
Texture2D<float4> blurTexture2_ : register(t2);
Texture2D<float4> blurTexture3_ : register(t3);
SamplerState sampler_ : register(s0);

struct Param {
    float intensity;
};
ConstantBuffer<Param> param_ : register(b0);

struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};

PSOutput main(PSInput input) {
    PSOutput output;

    float4 bloom = float4(0.0f, 0.0f, 0.0f, 0.0f);
    bloom += blurTexture0_.Sample(sampler_, input.texcoord);
    bloom += blurTexture1_.Sample(sampler_, input.texcoord);
    bloom += blurTexture2_.Sample(sampler_, input.texcoord);
    bloom += blurTexture3_.Sample(sampler_, input.texcoord);
    bloom /= NUM_TEXTURES;
    bloom.a = 1.0f;

    output.color = bloom;
    
    return output;
}