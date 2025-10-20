struct Constant {
    float32_t4x4 worldViewProjectionMatrix;
    float32_t4 color;
};
ConstantBuffer<Constant> g_Constant : register(b0);

TextureCube<float32_t4> g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct PSInput {
    float32_t4 position : SV_POSITION;
    float32_t3 texcoord : TEXCOORD0;
};

struct PSOutput {
    float32_t4 color : SV_TARGET0;
};

PSOutput main(PSInput input) {
    PSOutput output;
    float32_t4 textureColor = g_Texture.Sample(g_Sampler, input.texcoord);
    output.color = textureColor * g_Constant.color;
    return output;
}