struct Constant {
    float32_t4x4 worldViewProjectionMatrix;
    float32_t4 color;
};
ConstantBuffer<Constant> g_Constant : register(b0);

struct VSInput {
    float32_t3 position : POSITION0;
};

struct VSOutput {
    float32_t4 position : SV_POSITION;
    float32_t3 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = mul(float32_t4(input.position, 1.0f), g_Constant.worldViewProjectionMatrix).xyww;
    output.texcoord = input.position.xyz;
    return output;
}